import requests

host = "127.0.0.1"
port = 80

file = open("../config.txt")

while True:
    line = file.readline()
    
    if not line:
        break
    
    if (line.find('"address":') != -1) :
        line = line.replace('"address":"', '')
        line = line.replace('"\n', '')
        host = line
    elif (line.find('"port":') != -1):
        line = line.replace('"port":"', '')
        line = line.replace('"\n', '')
        port = line
        
file.close()

path = 'https://' + host + ':' + port + '/test.html'
response = requests.get(path, verify=False)

if ('Transfer-Encoding' in response.headers):
    print('CHUNK_TRANSFER TEST : PASSED.')
else:
    print('CHUNK_TRANSFER TEST : FAILED.')
