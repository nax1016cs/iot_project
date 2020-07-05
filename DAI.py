# DAI2.py  -- new version of Dummy Device DAI.py, modified by tsaiwn@cs.nctu.edu.tw
# you can get from here:  https://goo.gl/6jtP41   ; Search dummy + iottalk  for other files
import time, DAN, requests, random 
import threading, sys

# ServerURL = 'http://Your_server_IP_or_DomainName:9999' #with no secure connection
# ServerURL = 'http://192.168.20.101:9999' #with no secure connection
ServerURL = 'https://3.iottalk.tw' #with SSL secure connection
# ServerURL = 'https://Your_DomainName' #with SSL connection  (IP can not be used with https)
Reg_addr = None #if None, Reg_addr = MAC address

mac_addr = 'C860008BD249_9795654898900'  # put here for easy to modify;;  the mac_addr in DAN.py is NOT used
# Copy DAI.py to DAI2.py and then modify the above mac_addr, then you can have two dummy devices
Reg_addr = mac_addr   # Otherwise, the mac addr generated in DAN.py will always be the same !

DAN.profile['dm_name']='Dummy_Device'   # you can change this but should also add the DM in server
DAN.profile['df_list']=['Dummy_Sensor', 'Dummy_Control']
DAN.profile['d_name']= '0516097_device_T' # None for autoNaming
DAN.device_registration_with_retry(ServerURL, Reg_addr)

# global gotInput, theInput
gotInput=False
theInput="haha"
allDead=False



while True:
    try:
        f = open('CoolTerm Capture 2020-07-01 20-31-43','r')
        lines = f.read().splitlines()
        score = int(lines[-1]) 
        DAN.push ('Dummy_Sensor', score,  score)
        time.sleep(0.5)

    except Exception as e:
        print(e)
  
print("Bye ! --------------", flush=True)
sys.exit( );