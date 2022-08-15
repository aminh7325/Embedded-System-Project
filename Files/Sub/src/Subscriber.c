#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mysql.h>
#include "MQTTClient.h"
#include <time.h>

#define ADDRESS     "localhost"
#define CLIENTID    "Receiver_End"

static int counterFace=0;
static int counterAudio=0;
static int counter=0;
static FILE *foc;

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}


int stringtoint(const char* s){
  int res = 0, fact = 1;
  if (*s == '-'){
    s++;
    fact = -1;
  };
  for (int check = 0; *s; s++){
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      res = res * 10 + d;

    };
  };
  return res * fact;
};

 void sendToDBFace(char *values)
 {
    MYSQL *conn = mysql_init(NULL);
    char buff[200];
    int Integer[1];
    char Message[27];
    int i = 0;
    int j = 0;
    while(i < 2) {
        char Mess[100];
        int k = 0;
        while(values[j] != ',' && values[j] != '\0') {
            Mess[k] = values[j];
            k++;
            j++;
        }
        if(values[j] == ',') {
            j++;
        }
        if(i != 1)
            Integer[i] = stringtoint(Mess);
        else {
            for(int y = 0; y < 27; y++) {
                if((y == 0)||(y == 26)) {
                    Message[y] = '\"';
                }
                else {
                    Message[y] = Mess[y - 1];
                }
            }
        }   
        k = 0;
        i++;
    }
    Message[27] = '\0';
    sprintf(buff, "INSERT INTO FaceT VALUES(%d, %d, %s)", counterFace, (int)Integer[0], Message);
    

    if (mysql_real_connect(conn, "localhost", "root", "aminh7325",
        "ProjectDB", 0, NULL, 0) == NULL)
    {
        finish_with_error(conn);
    }
    if (mysql_query(conn, buff))
    {
        finish_with_error(conn);
    }
    mysql_close(conn);

 }


 void sendToDBAudio(char *values)
 {
    MYSQL *conn = mysql_init(NULL);
    char buff[200];
    int Integer[1];
    char Message[27];
    int i = 0;
    int j = 0;
    while(i < 2) {
        char Mess[100];
        int k = 0;
        while(values[j] != ',' && values[j] != '\0') {
            Mess[k] = values[j];
            k++;
            j++;
        }
        if(values[j] == ',') {
            j++;
        }

        if(i != 1)
            Integer[i] = stringtoint(Mess);
 
        else {
            for(int y = 0; y < 27; y++) {
                if((y == 0)||(y == 26)) {
                    Message[y] = '\"';
                }
                else {
                    Message[y] = Mess[y - 1];
                }
      
            }
        }   
        k = 0;
        i++;
    }
    Message[27] = '\0';
    //printf("\n -- -- -- -- -- -- -- \n INT0: %d \n INT1: %d \n INT2: %d \nMessPlus: %s \n", (int)Integer[0], (int)Integer[1], (int)Integer[2], Message);
    sprintf(buff, "INSERT INTO AudioT VALUES(%d, %d, %s)", counterAudio, (int)Integer[0], Message);
    

    if (mysql_real_connect(conn, "localhost", "root", "aminh7325",
        "ProjectDB", 0, NULL, 0) == NULL)
    {
        finish_with_error(conn);
    }
    if (mysql_query(conn, buff))
    {
        finish_with_error(conn);
    }
    mysql_close(conn);

 }
 
int topicNamecropper(char *s){
int identify = 0;
    int i = 0;
    int j = 0;
    while(i < 1){
        while(s[j] != '/' ) {
        	j++;
    	}
   		if(s[j] == 'A'){
   		identify = 1;
   		}
   		if(s[j] == 'F'){
   		identify = 2;
   		}
    	i++;
    }
    return identify;
}
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    int identify;
    counter++;

    time_t rawtime;
    struct tm * timeinfo;
    foc = fopen("log.txt", "a");

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    fprintf(foc, "%s : %s\n", asctime(timeinfo), payload);

    printf("%d)Received operation %s\n", counter, payload); 
    identify = topicNamecropper(topicName);
    if (strcmp(topicName , "sensors/Audio") == 0){
    counterAudio++;
    sendToDBAudio(payload);
    }
    if (strcmp(topicName , "sensors/Faces") == 0){
    counterFace++;
    sendToDBFace(payload);
    }

    fclose(foc);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}




int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = "aminh7325";
    conn_opts.password = "AminHaji";

    MYSQL *con = mysql_init(NULL);
    foc = fopen("log.txt", "w");
    fprintf(foc, "Time: Faces , Temp , Load , Data&Time\n");
    fclose(foc);

    if (mysql_real_connect(con, "localhost", "root", "aminh7325",
        "ProjectDB", 0, NULL, 0) == NULL)
    {
        finish_with_error(con);
    }

    if (mysql_query(con, "DROP TABLE IF EXISTS FaceT")) {
        finish_with_error(con);
      }

    if (mysql_query(con, "CREATE TABLE FaceT(id INT, Faces INT, Date_Time TEXT)")) {
        finish_with_error(con);
    }
    if (mysql_query(con, "DROP TABLE IF EXISTS AudioT")) {
        finish_with_error(con);
      }

    if (mysql_query(con, "CREATE TABLE AudioT(id INT, AudioDB INT, Date_Time TEXT)")) {
        finish_with_error(con);
    }
    mysql_close(con);

    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    //listen for operation
    MQTTClient_subscribe(client, "sensors/+", 0);
 
    for (;;) {
        sleep(1);
    }
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}

