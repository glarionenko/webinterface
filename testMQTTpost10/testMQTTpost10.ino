#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
EthernetClient ethClient;
PubSubClient client(ethClient);
/* НАСТРОЙКИ СЕТИ */
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 111);//мой айпи адрес
IPAddress server(188, 242, 123, 156);//внешний айпи сервера
IPAddress gateway(192, 168, 0, 1);//router
IPAddress myip;//айпи устройства есл и dhcp
/* НАСТРОЙКИ СЕТИ КОНЕЦ*/
/*
 * КОНТРОЛЛЕР ПИШЕТ В СТАТУСЫ
 * СЕРВЕР ПИШЕТ В ТРИГГЕРЫ
 * 
 */
//Количество игровых и неигровых элементов, которыми необходимо управлять
const int numberOfTasks = 5;//количество заданий

unsigned long started_at;//когда началась игра
int game_started;//статус игры( их 3)
//boolean game_power;

//Текущее стостояние загадки решена\не решена - изменяется контроллером
byte statesGame[numberOfTasks];
//Предыдущее стостояние загадки решена\не решена - изменяется контроллером
byte last_statesGame[numberOfTasks];
/*
    Предлагаю использовать триггеры для решения задачи
    Т.е. триггер будет читаться кодом и, если надо, затираться.
    Смысл триггера заключается в описании воздействия интерфейса и игровой логики на дальнейший код
*/
byte statesTriggers[numberOfTasks];//триггеры заданий, сбрасываются после отработки

//Имена топиков по количеству numberOfTasks
//возвращает имя топика по номеру задания

char* get_name(int num) {
  char* topic1;
  switch (num) {
    case 0:
      topic1 = "podpol/tasks/states/task1";
      break;
    case 1:
      topic1 = "podpol/tasks/states/task2";
      break;
    case 2:
      topic1 = "podpol/tasks/states/task3";
      break;
    case 3:
      topic1 = "podpol/tasks/states/task4";
      break;
    case 4:
      topic1 = "podpol/tasks/states/task5";
      break;
    default:
      topic1 = "error";
      break;
  }
  return topic1;
}
char* get_name2(int num) {
  char* topic2;
  switch (num) {
    case 0:
      topic2 = "podpol/tasks/triggers/task1";
      break;
    case 1:
      topic2 = "podpol/tasks/triggers/task2";
      break;
    case 2:
      topic2 = "podpol/tasks/triggers/task3";
      break;
    case 3:
      topic2 = "podpol/tasks/triggers/task4";
      break;
    case 4:
      topic2 = "podpol/tasks/triggers/task5";
      break;
    default:
      topic2 = "error";
      break;
  }
  return topic2;
}
/* Отправка статусов игры в MQTT */
boolean send_states(boolean start) {//
  boolean wasSent = 0;
  if(!start){
  for (int i = 0; i < numberOfTasks; i++) {
    if (statesGame[i] != last_statesGame[i]) {
      
      //Отправить, когда не равно предыдущему
      char* topicName = get_name(i);
      if(statesGame[i]==1){
      client.publish(topicName, "1");
      }else{
      client.publish(topicName, "0");
      }
      // сохранить в last_statesGame новые статусы statesGame
      last_statesGame[i] = statesGame[i];
    }
  }
  }else{
    for (int i = 0; i < numberOfTasks; i++) {

      char* topicName = get_name(i);
      if(statesGame[i]==1){
      client.publish(topicName, "1");
      }else{
      client.publish(topicName, "0");
      }
      // сохранить в last_statesGame новые статусы statesGame
      last_statesGame[i] = statesGame[i];
    
    }
  }
  return wasSent;
}
void compare_states_and_send() {
  //общее сравнение, а затем в send_ проверяю каждое задание и отправляю, если надо
  if (statesGame != last_statesGame) {
    send_states(0);
  }
}
/* Отправка статусов игры КОНЕЦ*/
//режим игры старт стоп сборка
void sendGameState(){
  if(game_started==0){
     client.publish("podpol/states/game_now_state","0");
      }
      if(game_started==1){
     client.publish("podpol/states/game_now_state","1");
      }
      if(game_started==2){
     client.publish("podpol/states/game_now_state","2");
      }
  }

void callback(char* topic, byte* payload, unsigned int length) {
  /* ДЕЛАЕМ СЛЕДУЮЩЕЕ, КОГДА ПОЛУЧЕНЫ ДАННЫе
      можно выбрать топик на какой что деалем
      Serial.print(topic);
      можно брать байты из содержимого топика payload
      Отправить байты можно так:
      byte sendIt[3]={0b110000,0b110001,0b110010};
      client.publish("bytesTopic",sendIt,3);
      Вывести сообщение так:
        for (int i=0;i<length;i++) {
        Serial.print((char)payload[i]);}

  */
  /*Логика приема сигналов с триггеров*/
String str(topic);
 Serial.println(str);
  for(int b=0;b<numberOfTasks;b++){
    String str2(get_name2(b));
 if(str.equals(str2)){
  if(payload[0]==49){
    statesTriggers[b]=1;
    }else{
      statesTriggers[b]=0;
      }
  }
}
  if(str.equals("podpol/states/game_set_state")){
  if(payload[0]==49){
    game_started=1;
    started_at=millis();
    }
    if(payload[0]==48){
    game_started=0;
    }
    if(payload[0]==50){
    game_started=2;
    }
    sendGameState();
  }
  if(str.equals("podpol/states/update")||str.equals("$SYS/broker/clients/total")){
  sendGameState();
  send_states(1);
  }
  /*Логика приема сигналов с триггеров КОНЕЦ*/

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (client.connect("arduinoClient", "openhabian", "55566678")) {
      client.publish("controller_one/online", "yes");
      client.subscribe("inTopic");
      client.subscribe("podpol/states/update");
      client.subscribe("$SYS/broker/clients/total");
      /*Топики триггеров*/
      for(int i=0;i<numberOfTasks;i++){
      client.subscribe(get_name2(i));
      client.publish("controller_one/task", i);
      }
      /*Топики триггеров конец*/
       client.subscribe("podpol/states/game_set_state");
       
       char bufIP[60];
  sprintf(bufIP, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  client.publish("controller_one/ip_my", bufIP);
      send_states(1);
    } else {
      //retry to connect
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
/* Настройка пинов */

/* Настройка пинов 2*/

void setup()
{
  /*Настройка пинов игры */
  //входы - аналог А0-А4
  //выходы - 2 - 6
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(2,LOW);
  digitalWrite(3,LOW);
  digitalWrite(4,LOW);
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  pinMode(A0,INPUT_PULLUP);
  pinMode(A1,INPUT_PULLUP);
  pinMode(A2,INPUT_PULLUP);
  pinMode(A3,INPUT_PULLUP);
  pinMode(A4,INPUT_PULLUP);
  /*Настройка пинов игры КОНЕЦ */
  Serial.begin(57600);

  Ethernet.begin(mac);
  myip = Ethernet.localIP();
  // Allow the hardware to sort itself out
  delay(1500);
  //Устанавливаем сервер и функцию, которая вызывается при обновлении топика, на который подписан
  client.setServer(server, 1883);
  client.setCallback(callback);
}
unsigned long started1[numberOfTasks];
unsigned long started2;

unsigned long sent_timer;

void sendTimer(){
  if(game_started==1){
  if((millis()-sent_timer)>1000){
    int game_time=(millis()-started_at)/1000;
    char bufTime[60];
    sprintf(bufTime, "%d",game_time+1500);
    client.publish("podpol/states/time",bufTime);
    sent_timer=millis();
    }
  }
  }
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
  //stringSend
  
  sendTimer();
  send_states(0);

//квест
tasks();
 
}
int tasksPins[5]={A0,A1,A2,A3,A4};
int tasksPinsOut[5]={2,3,4,5,13};
void tasks(){
   //task one
   for(int p=0;p<numberOfTasks;p++){
  if((digitalRead(tasksPins[p])==0)||(statesTriggers[p]==1)){
    digitalWrite(tasksPinsOut[p],HIGH);
    Serial.println("Button1");
    started1[p]=millis();
    statesTriggers[p]=0;
    statesGame[p]=1;
    }else{
      if((millis()-started1[p])>2000){
        digitalWrite(tasksPinsOut[p],LOW);
        statesGame[p]=0;
        }
      }
      }
      //
  }
