import socket

class TCP_Client(object):
    chunk_size = 200

    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def __enter__(self):
        self.socket.connect((self.host, self.port))
        return self

    def __exit__(self, *args):
        self.socket.close()

    def send(self, message):
        self.socket.send(message + '\r\n')

    def send_and_get(self, message):
        self.socket.send(message + '\r\n')
        return self.receive()

    def receive(self):
        data = ''
        while True:
            chunk = self.socket.recv(self.chunk_size)
            if chunk:
                data += chunk
                if '\r\n' in chunk:
                    break
            else:
                break
        return data.replace('\r\n', '')


class SMTP_Client(TCP_Client):
    def __enter__(self):
        super(SMTP_Client, self).__enter__()
        test_smtp_output('welcome', self.receive(), ['220', 'ready'])
        return self

    def hello(self, cmd='HELO', name='max.ru'):
        return self.send_and_get('{} {}'.format(cmd, name))

    def quit(self, cmd='QUIT'):
        return self.send_and_get('{}'.format(cmd))

    def hello_extended(self, cmd='EHLO', name='max.ru'):
        return self.send_and_get('{} {}'.format(cmd, name))

    def mail_from(self, cmd='MAIL FROM', email='<chesalka@mail.ru>'):
        return self.send_and_get('{}:{}'.format(cmd, email))

    def recipient_to(self, cmd='RCPT TO', email='<max@mail.ru>'):
        return self.send_and_get('{}:{}'.format(cmd, email))

    def data(self, cmd='DATA', message='Hello, friend\n.'):
        self.send('{}'.format(cmd))
        test_smtp_output('wait text', self.receive(), ['354', 'start', '<CRLF>.<CRLF>'])
        for line in message.split('\n'):
            self.send('{}'.format(line))
        return self.receive()

    def no_operation(self, cmd='NOOP'):
        return self.send_and_get('{}'.format(cmd))

    def reset(self, cmd='RSET'):
        return self.send_and_get('{}'.format(cmd))

    def verify(self, cmd='VRFY', email_part='chesal'):
        return self.send_and_get('{} {}'.format(cmd, email_part))


def test_smtp_output(tname, response, must_contain):
    for string in must_contain:
        if string.lower() not in response.lower():
            raise Exception('- {} test failed. ["{}" not occured in "{}"]'.format(tname, string, response))
    
    print('+ {} test succeeded.'.format(tname.capitalize()))


HOST = 'localhost'
PORT = 9091

def script_1():
    print '=== Script 1 started (Test connection - HELO, QUIT, then write) ==='

    with SMTP_Client(HOST, PORT) as client:
        test_smtp_output('hello', client.hello(), ['250', 'ok'])
        test_smtp_output('quit', client.quit(), ['221', 'closing'])
        try:
            client.hello()
        except Exception as e:
            test_smtp_output('after close', str(e), ['errno 54', 'connection reset'])
    print ''

def script_2():
    print '=== Script 2 started (Test EHLO, RSET, NOOP commands) ==='

    with SMTP_Client(HOST, PORT) as client:
        test_smtp_output('1st ehlo', client.hello_extended(), ['250-', 'pipelining', '8bitmime', 'vrfy'])
        test_smtp_output('1st noop', client.no_operation(), ['250', 'ok'])
        test_smtp_output('rset', client.reset(), ['250', 'ok'])
        test_smtp_output('hello', client.hello(), ['250', 'ok'])
        test_smtp_output('2nd noop', client.no_operation(), ['250', 'ok'])
        test_smtp_output('2nd ehlo', client.hello_extended(), ['250-', 'pipelining', '8bitmime', 'vrfy'])
        test_smtp_output('quit', client.quit(), ['221', 'closing'])
    print ''

def script_3():
    print '=== Script 3 started === (Test ordinal mailing)'

    with SMTP_Client(HOST, PORT) as client:
        test_smtp_output('hello', client.hello(), ['250', 'ok'])
        test_smtp_output('sender', client.mail_from(), ['250', 'ok'])
        test_smtp_output('receiver', client.recipient_to(), ['250', 'ok'])
        test_smtp_output('data', client.data(), ['250', 'ok'])
        test_smtp_output('quit', client.quit(), ['221', 'closing'])
    print ''

def script_4():
    print '=== Script 4 started === (Test many recipients)'

    with SMTP_Client(HOST, PORT) as client:
        test_smtp_output('hello', client.hello(), ['250', 'ok'])
        test_smtp_output('sender', client.mail_from(), ['250', 'ok'])
        
        test_smtp_output('receiver', client.recipient_to(email='<test1@mail.ru>'), ['250', 'ok'])
        test_smtp_output('receiver', client.recipient_to(email='<test2@mail.ru>'), ['250', 'ok'])
        test_smtp_output('receiver', client.recipient_to(email='<test3@mail.ru>'), ['250', 'ok'])
        
        test_smtp_output('data', client.data(), ['250', 'ok'])
        test_smtp_output('quit', client.quit(), ['221', 'closing'])
    print ''

def script_5():
    print '=== Script 5 started === (Errors 5xx)'

    with SMTP_Client(HOST, PORT) as client:
        test_smtp_output('mail_from before hello', client.mail_from(), ['503', 'bad', 'sequence'])
        test_smtp_output('invalid hello', client.hello(cmd='HELLO'), ['500', 'invalid', 'command'])
        test_smtp_output('hello', client.hello(), ['250', 'ok'])

        test_smtp_output('invalid sender argument', client.mail_from(email='test'), ['501', 'invalid', 'argument'])
        test_smtp_output('unknown sender', client.mail_from(email='<test@mail.ru>'), ['550', 'unknown', 'sender'])
        test_smtp_output('sender', client.mail_from(), ['250', 'ok'])
        test_smtp_output('hello after mail_from', client.hello(), ['503', 'bad', 'sequence'])
        
        test_smtp_output('no receiver', client.recipient_to(email=''), ['501', 'invalid', 'argument'])
        test_smtp_output('quit', client.quit(), ['221', 'closing'])
    print ''

def script_6():
    print '=== Script 6 started === (Verify command)'

    with SMTP_Client(HOST, PORT) as client:
        test_smtp_output('hello', client.hello(), ['250', 'ok'])
        test_smtp_output('verify known', client.verify(), ['250', 'chesalin', 'denis', '<chesalka@mail.ru>'])
        test_smtp_output('verify unknown', client.verify(email_part='test@mail.ru'), ['550', 'unknown', 'sender'])
        test_smtp_output('quit', client.quit(), ['221', 'closing'])
    print ''

if __name__ == '__main__':
    try:
        script_1()
        script_2()
        script_3()
        script_4()
        script_5()
        script_6()

    except Exception as e:
        print(e)

