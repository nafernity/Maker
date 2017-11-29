/*
Magellan v1.42 NB-IoT Playgroud Platform .
support Quectel BC95 
NB-IoT with AT command

Author:   Phisanupong Meepragob
Create Date:     1 May 2017. 
Modified: 23 November 2017.

Released for private use.
*/

#include "Magellan.h"
//#include "AltSoftSerial.h"

//#include <MemoryFree.h>

//char apnName[] ="IOTURL";
//char serverIP[] = "103.20.205.104";   //DEV SERVER
char serverIP[] = "103.20.205.85";   //PRODUCTION SERVER
//char serverPort[] = "5683";
//char NBpath[]="NBIoT";
//char socketNo[] ="0";

AltSoftSerial myserial;

unsigned int Msg_ID=0;

char *pathauth;

//################### Buffer #######################
String input;
String buffer;
String send_buffer;
//################### counter value ################
byte k=0;

//################## flag ##########################
bool end=false;
bool send_NSOMI=false;
bool flag_rcv=true;

//################### Parameter ####################
long data_interval=10000;
unsigned int Latency=2000;
unsigned int send_cnt=0;
unsigned int resp_cnt=0;
byte maxtry=0;
byte cnt_retrans=0;
unsigned int resp_msgID=0;
bool en_send=true;
bool en_rcv=false;
bool getpayload=false;
//unsigned long previousMillis = 0;
unsigned long previous=0;
bool Created=false;
bool sendget=false;
byte cnt_retrans_get=0;
unsigned int previous_send=0;
unsigned int previous_get=0;
unsigned int get_ID=0;
bool NOTFOUND=false;
bool GETCONTENT=false;
bool RCVRSP=false;
String rcvdata="";
byte cnt_error=0;
unsigned int wait_time=2;
bool success=false;
unsigned int auth_len=0;

void event_null(char *data){}

Magellan::Magellan()
{
  Event_debug =  event_null;
}

AIS_BC95_RES Magellan:: wait_rx_bc(long tout,String str_wait)
{
  unsigned long pv_ok = millis();
  unsigned long current_ok = millis();
  String input;
  unsigned char flag_out=1;
  unsigned char res=-1;
  AIS_BC95_RES res_;
  res_.temp="";
  res_.data = "";

  while(flag_out)
  {
    if(_Serial->available())
    {
      input = _Serial->readStringUntil('\n');
      
      res_.temp+=input;
      if(input.indexOf(str_wait)!=-1)
      {
        res=1;
        flag_out=0;
      } 
        else if(input.indexOf(F("ERROR"))!=-1)
      {
        res=0;
        flag_out=0;
      }
    }
    current_ok = millis();
    if (current_ok - pv_ok>= tout) 
    {
      flag_out=0;
      res=0;
      pv_ok = current_ok;
    }
  }
  
  res_.status = res;
  res_.data = input;
  return(res_);
}

//################################ Magellan platform #######################################
void Magellan::begin(char *auth)
{
  myserial.begin(9600);
  _Serial = &myserial;
  Serial.println(F("               AIS BC95 NB-IoT Shield V1.42"));
  Serial.println(F("                  Magellan IoT Platform"));
  Serial.println(F("##################### Reset Module #####################"));

  Serial.println(F("################ Welcome NB-IoT network ################"));
  Serial.println(F(">>Reboot Module ...."));
  _Serial->println(F("AT+NRB"));
  //delay(6000);
  AIS_BC95_RES res = wait_rx_bc(6000,F("OK"));
  if(res.status)
  {
    Serial.println(res.data);
  }
  delay(5000);
  _Serial->println(F("AT"));
  res = wait_rx_bc(1000,F("OK"));
  _Serial->println(F("AT"));
  res = wait_rx_bc(1000,F("OK"));
  _Serial->println(F("AT"));
  res = wait_rx_bc(1000,F("OK"));

  if(debug) Serial.println(F(">>Setup CFUN"));
  _Serial->println(F("AT+CFUN=1"));
  delay(6000);
  

/*  res = wait_rx_bc(5000,F("OK"));
  if(res.status)
  {
    Serial.println(res.data);
  }*/
  
/*  Serial.println(F(">>Setup Network"));
  _Serial->println(F("AT+CGDCONT=1,\"IP\",\"IOTURL\""));
  delay(6000);*/
  
/*  res = wait_rx_bc(5000,F("OK"));
  if(res.status)
  {
    Serial.println(res.data);
  }
*/
  _Serial->println(F("AT+NCONFIG=AUTOCONNECT,TRUE"));
  delay(6000);

  Serial.println(F(">>Connecting to network"));
  _Serial->println(F("AT+CGATT=1"));
  delay(6000);
  /*
  res = wait_rx_bc(5000,F("OK"));
  if(res.status)
  {
    Serial.println(res.data);
  }
  */
  Serial.println(F(">>Create socket"));
  _Serial->println(F("AT+NSOCR=DGRAM,17,5684,1"));
  delay(6000);
  /*
  res = wait_rx_bc(5000,F("OK"));
  if(res.status)
  {
    Serial.println(res.data);
  }
  */

  /*
  res = wait_rx_bc(5000,F("OK"));
  if(res.status)
  {
    Serial.println(res.data);
  }
  */
/*  Serial.println(F(">>Check signal"));
  //_Serial->println(F("AT+CSQ=?"));
  //delay(5000);
  _Serial->println(F("AT+CSQ"));
  res = wait_rx_bc(500,F("+CSQ"));*/
  //signal sig;
/*  int x = 0;
  String tmp;
  if(res.status)
  {
    //+CSQ:99,99
    if(res.data.indexOf(F("+CSQ"))!=-1)
    {
      int index = res.data.indexOf(F(":"));
      int index2 = res.data.indexOf(F(","));
      tmp = res.data.substring(index+1,index2);
      
      if (tmp == F("99"))
      {
        //sig.csq = F("N/A");
        //sig.rssi = F("N/A");
        Serial.print(F(">> Signal csq: "));
        Serial.println(F("N/A"));
        Serial.print(F(">> Signal rssi: "));
        Serial.println(F("N/A"));
      }
      else
      {
        //sig.csq = tmp;
        x = tmp.toInt();
        x = (2*x)-113;
        Serial.print(F(">>Signal csq: "));
        Serial.println(tmp);
        Serial.print(F(">>Signal rssi: "));
        Serial.println(String(x));
        //sig.rssi = String(x);
      }
      //sig.ber  = res.data.substring(index2+1);
    }
  }
  res = wait_rx_bc(500,F("OK"));*/

  //Serial.println(F(">>Ping test"));
  //_Serial->print(F("AT+NPING="));
  //_Serial->println(serverIP);
  
  //_Serial->println(F("AT+NPING=103.20.205.103"));

  //res = wait_rx_bc(3000,F("+NPING"));
/*  if(res.status)
  {
    Serial.println(res.data);

    String data = res.data;
    int index = data.indexOf(F(":"));
    int index2 = data.indexOf(F(","));
    int index3 = data.indexOf(F(","),index2+1);
    //pingresp.status = true;       
    //pingresp.addr = data.substring(index+1,index2);
    //pingresp.ttl = 
    Serial.print("TTL :");
    Serial.println(data.substring(index2+1,index3));
    Serial.print("RTT :");
    Serial.println(data.substring(index3+1,data.length()));  
  }
  res = wait_rx_bc(500,F("OK"));*/
  // unsigned int cnt_resp=0;
  // while(cnt_resp<3000){
  //     _Serial->println(F("AT+NSORF=0,100"));
  //     res = wait_rx_bc(1000,F("OK"));
  //     if(res.status)
  //     {
  //           Serial.println(res.data);

  //     }
  //     send_NSOMI=true;

  //     waitResponse();
  //     cnt_resp++;
      

  // }

  pathauth=auth;

  cnt_auth_len(pathauth,auth_len);

  previous=millis();
}

void Magellan::printHEX(char *str)
{
  char *hstr;
  hstr=str;
  char out[3]="";
  int i=0;
  bool flag=false;
  while(*hstr)
  {
    flag=itoa((int)*hstr,out,16);
    
    if(flag)
    {
      _Serial->print(out);

      if(debug)
      {
        Serial.print(out);
      }
      
    }
    hstr++;
  }
}

void Magellan::printmsgID(unsigned int messageID)
{
  char Msg_ID[3];
  
  utoa(highByte(messageID),Msg_ID,16);
  if(highByte(messageID)<16)
  {
    if(debug) Serial.print(F("0"));
    _Serial->print(F("0"));
    if(debug) Serial.print(Msg_ID);
    _Serial->print(Msg_ID);
  }
  else
  {
    _Serial->print(Msg_ID);
    if(debug)  Serial.print(Msg_ID);
  }

  utoa(lowByte(messageID),Msg_ID,16);
  if(lowByte(messageID)<16)
  {
    if(debug)  Serial.print(F("0"));
    _Serial->print(F("0"));
    if(debug)  Serial.print(Msg_ID);
    _Serial->print(Msg_ID);
  }
  else
  {
    _Serial->print(Msg_ID);
    if(debug)  Serial.print(Msg_ID);
  }
} 

void Magellan::print_pathlen(unsigned int path_len)
{   
    unsigned int extend_len=0;
    if(path_len>13){
      extend_len=path_len-13;

      char extend_L[3];
      itoa(lowByte(extend_len),extend_L,16);
      _Serial->print(F("d"));
      _Serial->print(extend_L);
      if(debug) Serial.print(F("d"));
      if(debug) Serial.print(extend_L);
    }
    else{
      if(path_len<=9)
      {
        char hexpath_len[2]="";   
      sprintf(hexpath_len,"%i",path_len);
      _Serial->print(hexpath_len);
      if(debug) Serial.print(hexpath_len);
      
      }
      else
      {
        if(path_len==10) 
        {
          _Serial->print(F("a"));
          if(debug) Serial.print(F("a"));
        }
        if(path_len==11)
        {
          _Serial->print(F("b"));
          if(debug) Serial.print(F("b"));
        } 
        if(path_len==12)
        {
          _Serial->print(F("c"));
          if(debug) Serial.print(F("c"));
        } 
        if(path_len==13)
        {
          _Serial->print(F("d"));
          if(debug) Serial.print(F("d"));
        } 
        
      }
   }  
}

void Magellan::cnt_auth_len(char *auth,unsigned int len_auth){
  char *hstr;
  hstr=auth;
  char out[3]="";
  int i=0;
  bool flag=false;
  while(*hstr)
  {
    len_auth++;
    hstr++;
  }
}

String Magellan::post(String payload)
{
  success=false;
  while(!success){
    send(payload);
    waitResponse();
    //Serial.println(success);
  }
  return buffer;
}

void Magellan::send(String payload,int style=1,int interval=20)
{
  AIS_BC95_RES res;
  
  unsigned int currenttime=millis();


  if(!flag_rcv && cnt_retrans>=4){
    Serial.println(F("Timeout"));
    //_Serial->println(F("AT+NPING=8.8.8.8"));
    _Serial->println(F("AT"));
    res = wait_rx_bc(100,F("OK"));
/*    _Serial->println(F("AT"));
    res = wait_rx_bc(1000,F("OK"));
    _Serial->println(F("AT"));
    res = wait_rx_bc(1000,F("OK"));*/
    if(res.status)
    {
      Serial.println(res.data);
    }
  }

  if(flag_rcv || cnt_retrans>=4)
  {
    Serial.println(F("Load new payload"));
    send_buffer=payload;
    Msg_ID++;

    cnt_retrans=0;
    en_rcv=false;
    en_send=true;
    wait_time=2;
    
  }
  else{


    en_rcv=true;
    unsigned int Difftime=currenttime-previous_send;
    if(Difftime>=wait_time*1000){

      Serial.print(F("Difftime :"));
      Serial.println(Difftime);

      Serial.print(F("wait_time :"));
      Serial.println(wait_time*1000);

      Serial.print(F("Retransmit :"));
      Serial.println(cnt_retrans+1);

      cnt_retrans++;
      previous_send=currenttime;
      en_send=true;
    }
    else{
      en_send=false;
    }


    if(cnt_retrans>=4){
      success=true;
    }

    
    
  }

  char data[send_buffer.length()+1]="";
  send_buffer.toCharArray(data,send_buffer.length()+1);
  
  if(debug) Serial.println(resp_msgID);

  if(en_send){

      //_Serial->println(F("AT"));
      //res = wait_rx_bc(1000,F("OK"));
      send_cnt++;
      Serial.print(F("post cnt :"));
      Serial.println(send_cnt);
      Serial.print(F("--->>> post data : Msg_ID "));
      Serial.print(Msg_ID);
      Serial.print(" ");
      Serial.print(send_buffer);
      //NBserial.print(F("AT+NSOST=0,52.74.30.1,5683,"));
      _Serial->print(F("AT+NSOST=0,"));
      _Serial->print(serverIP);
      _Serial->print(F(",5683,"));
      //_Serial->print(F("AT+NSOST=0,103.20.205.103,5683,"));     //Dev server
      //_Serial->print(F("AT+NSOST=0,103.20.205.85,5683,"));       //Production server
      //Serial.print(F("AT+NSOST=0,103.20.205.103,5683,"));
      if(debug) Serial.print(F("AT+NSOST=0,")); 
      if(debug) Serial.print(serverIP);
      if(debug) Serial.print(F(",5683,")); 
      //if(debug) Serial.print(F("AT+NSOST=0,103.20.205.103,5683,"));   //Dev server
      //if(debug) Serial.print(F("AT+NSOST=0,103.20.205.85,5683,"));     //Production server
      Serial.println(auth_len);
      if(debug) Serial.print(49+send_buffer.length());
      _Serial->print(49+send_buffer.length());
      //Serial.print(F("4002"));
      if(debug) Serial.print(F(",4002"));
      _Serial->print(F(",4002"));/**/
      printmsgID(Msg_ID);
      _Serial->print(F("b54e42496f54"));
      if(debug) Serial.print(F("b54e42496f54"));
      _Serial->print(F("0d17"));
      if(debug) Serial.print(F("0d17"));
      printHEX(pathauth);
      _Serial->print(F("ff"));  //49
      if(debug) Serial.print(F("ff"));
      printHEX(data);  
      _Serial->println();

      Serial.println();
      
      //Serial.println(cnt_retrans);
      switch (cnt_retrans) {
        case 1:
          // statements
          wait_time=4;
          break;
        case 2:
          // statements
          wait_time=8;
          break;
        case 3:
          // statements
          wait_time=16;
          break;
        case 4:
          // statements
          wait_time=32;
          break;
        //default:
          // statements
      }
/*      for(byte l=1;l<=cnt_retrans;l+=1)
      {
        wait_time=wait_time*2;
        
      } */
      Serial.println(wait_time*1000);
      
      //res = wait_rx_bc(1000,F("OK"));

      //if(res.status)
      //{
      //  Serial.println(res.data);
      //}
      flag_rcv=false;

      previous_send=millis();
  }
  //Serial.print("freeMemory()=");
  //Serial.println(freeMemory());
/*  delay(5000);
  _Serial->println(F("AT+NSORF=0,100"));
  res = wait_rx_bc(1000,F("OK"));
  if(res.status)
  {
        Serial.println(res.data);
  }
  send_NSOMI=true;*/
      
}


String Magellan::get(String Resource){
  
  AIS_BC95_RES res;

  char data[Resource.length()+1]="";
  Resource.toCharArray(data,Resource.length()+1); 

  //Serial.println(buffer);

  // if(Created && (buffer.length()>0) && !(buffer.indexOf(F("ERROR")))){

  //   send("{\""+Resource+"\":"+buffer+"}");
  // }
/*  else{
    send("{\""+Resource+"\":0}");
  }*/
  if(!Created){
    if(flag_rcv){
      previous_send=millis();
      Created=true;
    }
    String resource_data=",\""+Resource+"\":0";
    //post("{\""+Resource+"\":0}");
    send_buffer.remove(0,1);
    send_buffer.remove(send_buffer.length()-1,send_buffer.length());
    send_buffer=send_buffer+resource_data;
    send_buffer="{"+send_buffer+"}";

    char data[send_buffer.length()+1]="";
    send_buffer.toCharArray(data,send_buffer.length()+1);
    _Serial->print(F("AT+NSOST=0,"));
    _Serial->print(serverIP);
    _Serial->print(F(",5683,"));
    _Serial->print(49+send_buffer.length());
    _Serial->print(F(",4002"));
    printmsgID(Msg_ID);
    _Serial->print(F("b54e42496f54"));
    _Serial->print(F("0d17"));
    printHEX(pathauth);
    _Serial->print(F("ff"));  //49
    printHEX(data);  
    _Serial->println();

    send_buffer.remove(send_buffer.length()-resource_data.length(),send_buffer.length());
    //  if(flag_rcv) Created=true;


  }
  else{
      if(sendget){
        if(NOTFOUND){
          post("{\""+Resource+"\":0}");
        }
        if(!getpayload && cnt_retrans_get<=4)
        {
          cnt_retrans_get++;
          en_rcv=true;
          //flag_rcv=false;
        }
        else if(getpayload)
        {
          Serial.print(F("get ID="));
          Serial.println(get_ID);
          Serial.print(F("resp ID="));
          Serial.println(resp_msgID);

          Serial.print("buffer=");
          Serial.println(rcvdata);
          
          getpayload=false;
          sendget=false;
          cnt_error=0;

          unsigned int currentms=millis();
          if(currentms-previous_send>=20000){
            
            //post("{\""+Resource+"\":"+rcvdata+"}");
            String resource_data=",\""+Resource+"\":"+rcvdata;
            send_buffer.remove(0,1);
            send_buffer.remove(send_buffer.length()-1,send_buffer.length());
            send_buffer=send_buffer+resource_data;
            send_buffer="{"+send_buffer+"}";

            char data[send_buffer.length()+1]="";
            send_buffer.toCharArray(data,send_buffer.length()+1);
            _Serial->print(F("AT+NSOST=0,"));
            _Serial->print(serverIP);
            _Serial->print(F(",5683,"));
            _Serial->print(49+send_buffer.length());
            _Serial->print(F(",4002"));
            printmsgID(Msg_ID);
            _Serial->print(F("b54e42496f54"));
            _Serial->print(F("0d17"));
            printHEX(pathauth);
            _Serial->print(F("ff"));  //49
            printHEX(data);  
            _Serial->println();
            send_buffer.remove(send_buffer.length()-resource_data.length(),send_buffer.length());

            previous_send=currentms;
          }

            

          //send("{\""+Resource+"\":"+rcvdata+"}");
        }
        else if(cnt_retrans_get>=4){
            en_rcv=false;
            getpayload=false;
            sendget=false;
            cnt_retrans_get=0;
        }
      }


      if(!sendget){
        Msg_ID++;
        get_ID=Msg_ID;
        //_Serial->print(F("AT+NSOST=0,103.20.205.85,5683,"));       //Production server
        //if(debug) Serial.print(F("AT+NSOST=0,103.20.205.85,5683,"));     //Production server
        //_Serial->print(F("AT+NSOST=0,103.20.205.103,5683,"));     //Dev server
        //if(debug) Serial.print(F("AT+NSOST=0,103.20.205.103,5683,"));   //Dev server
        Serial.print(F("--->>> GET data : Msg_ID "));
        Serial.print(Msg_ID);
        Serial.print(" ");
        Serial.print(Resource);
        
        _Serial->print(F("AT+NSOST=0,"));
        _Serial->print(serverIP);
        _Serial->print(F(",5683,"));

        if(debug) Serial.print(F("AT+NSOST=0,")); 
        if(debug) Serial.print(serverIP);
        if(debug) Serial.print(F(",5683,")); 

        unsigned int path_len=Resource.length();
        if(path_len>13){
          if(debug) Serial.print(52+path_len);
          _Serial->print(52+path_len);
        }
        else{
          if(debug) Serial.print(51+path_len);
          _Serial->print(51+path_len);
        }

        if(debug) Serial.print(F(",4001"));
        _Serial->print(F(",4001"));
        printmsgID(Msg_ID);
        _Serial->print(F("b54e42496f54"));
        if(debug) Serial.print(F("b54e42496f54"));
        _Serial->print(F("0d17"));
        if(debug) Serial.print(F("0d17"));
        printHEX(pathauth);

        _Serial->print(F("4"));
        if(debug) Serial.print(F("4"));

        print_pathlen(path_len);
        printHEX(data);

        _Serial->print(F("8106"));
        if(debug) Serial.print(F("8106"));

        _Serial->println();

        Serial.println();

        //Serial.print("freeMemory()=");
        //Serial.println(freeMemory());
        sendget=true;
        res = wait_rx_bc(1000,F("ERROR"));
        if(res.status)
        {
            //Serial.println(res.data);
            //begin();
          cnt_error++;
        }
        //delay(100);
        if(cnt_error>=50) begin(pathauth);
      }
  }


  return rcvdata;


}

//############################### Response ######################################

void Magellan::print_rsp_header(const String Msgstr)
{
  //unsigned int number = (unsigned int) strtol( &Msgstr.substring(4,8)[0], NULL, 16);
  resp_msgID = (unsigned int) strtol( &Msgstr.substring(4,8)[0], NULL, 16);

  //print_rsp_Type(Msgstr.substring(0,2),number);
  print_rsp_Type(Msgstr.substring(0,2),resp_msgID);

   switch((int) strtol(&Msgstr.substring(2,4)[0], NULL, 16))
   {
      case CREATED: 
                    NOTFOUND=false;
                    GETCONTENT=false;
                    RCVRSP=true;
                    Serial.println(F("2.01 CREATED")); 
                    break;
      case DELETED: Serial.println(F("2.02 DELETED")); break;
      case VALID: Serial.println(F("2.03 VALID")); break;
      case CHANGED: Serial.println(F("2.04 CHANGED")); break;
      case CONTENT: 
                    NOTFOUND=false;
                    GETCONTENT=true;
                    RCVRSP=false;
                    Serial.println(F("2.05 CONTENT")); 
                    break;
      case CONTINUE: Serial.println(F("2.31 CONTINUE")); break;
      case BAD_REQUEST: Serial.println(F("4.00 BAD_REQUEST")); break;
      case FORBIDDEN: Serial.println(F("4.03 FORBIDDEN")); break;
      case NOT_FOUND: 
                    Serial.println(F("4.04 NOT_FOUND"));
                    GETCONTENT=false; 
                    NOTFOUND=true;
                    RCVRSP=false;
                    break;
      case METHOD_NOT_ALLOWED: 
                    RCVRSP=false;
                    Serial.println(F("4.05 METHOD_NOT_ALLOWED")); 
                    break;
      case NOT_ACCEPTABLE: Serial.println(F("4.06 NOT_ACCEPTABLE")); break;
      case REQUEST_ENTITY_INCOMPLETE: Serial.println(F("4.08 REQUEST_ENTITY_INCOMPLETE")); break;
      case PRECONDITION_FAILED: Serial.println(F("4.12 PRECONDITION_FAILED")); break;
      case REQUEST_ENTITY_TOO_LARGE: Serial.println(F("4.13 REQUEST_ENTITY_TOO_LARGE")); break;
      case UNSUPPORTED_CONTENT_FORMAT: Serial.println(F("4.15 UNSUPPORTED_CONTENT_FORMAT")); break;
      case INTERNAL_SERVER_ERROR: Serial.println(F("5.00 INTERNAL_SERVER_ERROR")); break;
      case NOT_IMPLEMENTED: Serial.println(F("5.01 NOT_IMPLEMENTED")); break;
      case BAD_GATEWAY: Serial.println(F("5.02 BAD_GATEWAY")); break;
      case SERVICE_UNAVAILABLE: Serial.println(F("5.03 SERVICE_UNAVAILABLE")); break;
      case GATEWAY_TIMEOUT: Serial.println(F("5.04 GATEWAY_TIMEOUT")); break;
      case PROXY_NOT_SUPPORTED: Serial.println(F("5.05 PROXY_NOT_SUPPORTED")); break;

      default : //Optional
                GETCONTENT=false;
   }

   Serial.print(F("MessageID:"));

   
   //Serial.println(number);
   Serial.println(resp_msgID);
}

void Magellan::print_rsp_Type(const String Msgstr,unsigned int msgID)
{
  if(Msgstr.indexOf(ack)!=-1)
  {
    Serial.println(F("Acknowledge"));
    flag_rcv=true;
    success=true;
  }
  if(Msgstr.indexOf(con)!=-1)
  {
    Serial.println(F("Confirmation"));
    Serial.println();
    resp_cnt++;
    Serial.print(F("Resp cnt :"));
    Serial.println(resp_cnt);
    flag_rcv=true;
    success=true;
  }
  if(Msgstr.indexOf(rst)!=-1)
  {
    Serial.println(F("Reset"));
    flag_rcv=true;
    success=true;
  }
  if(Msgstr.indexOf(non_con)!=-1)
  {
    Serial.println(F("Non-Confirmation"));
    flag_rcv=true;
    success=true;
  }
}

void Magellan::miniresponse(String rx)
{
  print_rsp_header(rx);


  String data_payload="";

  unsigned int indexff=rx.indexOf(F("FF"));

  if(rx.indexOf(F("FFF"))!=-1)
  {

      data_payload=rx.substring(indexff+3,rx.length());

      Serial.print(F("---<<< Response :"));
      buffer="";
      for(byte k=2;k<data_payload.length()+1;k+=2)
      {
        char str=(char) strtol(&data_payload.substring(k-2,k)[0], NULL, 16);
        Serial.print(str);

        if(GETCONTENT or RCVRSP){
          buffer+=str;
        }
        
  
      }
      if(GETCONTENT)
      {
        rcvdata=buffer;
        getpayload=true;
      } 
      Serial.println(F(""));

      
  }
  else
  {

      data_payload=rx.substring(indexff+2,rx.length());
      Serial.print(F("---<<< Response :"));
      buffer="";
      for(byte k=2;k<data_payload.length()+1;k+=2)
      {
        char str=(char) strtol(&data_payload.substring(k-2,k)[0], NULL, 16);
        Serial.print(str);

        if(GETCONTENT or RCVRSP){
          buffer+=str;
        }
      }
      if(GETCONTENT) {
        rcvdata=buffer;
        getpayload=true;
      } 
      Serial.println(F(""));

      
  }
  Serial.println(F("----------------------- End -----------------------"));
}

void Magellan:: waitResponse()
{ 
  unsigned long current=millis();
  if(en_rcv && (current-previous>=250) && !(_Serial->available()))
  {
      _Serial->println(F("AT+NSORF=0,100"));
      //AIS_BC95_RES res = wait_rx_bc(1000,F("OK"));
      //send_NSOMI=true;
      previous=current;
  }



  if(_Serial->available())
  {
    char data=(char)_Serial->read();
    if(data=='\n' || data=='\r')
    {
      if(k>2)
      {
        end=true;
        k=0;
      }
      k++;
    }
    else
    {
      input+=data;
    }
    if(debug) Serial.println(input); 
  }
  if(end){
      if(input.indexOf(F("+NSONMI:"))!=-1)
      {
          if(debug) Serial.print(F("send_NSOMI "));
          if(debug) Serial.println(input);
          if(input.indexOf(F("+NSONMI:"))!=-1)
          {
            if(debug) Serial.print(F("found NSONMI "));
            _Serial->println(F("AT+NSORF=0,100"));
            //AIS_BC95_RES res = wait_rx_bc(1000,F("OK"));
            input=F("");
            send_NSOMI=true;
            Serial.println();
            //Serial.println(F("==========================================================================="));
          }
          end=false;
      }

      else
        {
          if(debug) Serial.print(F("get buffer "));
          if(debug) Serial.println(input);
    /*      if(input.indexOf(F("+NSONMI:"))!=-1) 
          {   Serial.println(F("got NSOMI and set to false"));
              send_NSOMI=false;

            _Serial->println(F("AT+NSORF=0,100"));
            AIS_BC95_RES res = wait_rx_bc(1000,F("OK"));
            input=F("");
            send_NSOMI=true;
            Serial.println();
          }*/
          end=false;
          //if(input.indexOf(F("0,103.20.205.104"))!=-1)   //Dev server
          if(input.indexOf(F("0,103.20.205.85"))!=-1)   //Production server
          {
            int index1 = input.indexOf(F(","));
            if(index1!=-1) 
            {
              int index2 = input.indexOf(F(","),index1+1);
              index1 = input.indexOf(F(","),index2+1);
              index2 = input.indexOf(F(","),index1+1);
              index1 = input.indexOf(F(","),index2+1);
              if(debug) Serial.println(input.substring(index2+1,index1));
              //receive_data(input.substring(index2+1,index1));
              if(debug) Serial.println(F("--->>>  Response "));
              miniresponse(input.substring(index2+1,index1));

            }

            
            
          }
          send_NSOMI=false;
          input=F("");
          }       
        }


}



