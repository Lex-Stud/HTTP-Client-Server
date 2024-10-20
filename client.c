#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdbool.h>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

#include "helpers.h"
#include "parson.h"
#include "requests.h"

#define HOST "34.246.184.49"
#define PORT 8080

bool haveSpace(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] == ' ') {
      return true;
    }
  }

  return false;
}

bool validString(char *string) {
  if (strlen(string) == 0) {
    return false;
  }

  if (string[0] == '\n') {
    return false;
  }

  for (int i = 0; i < strlen(string); i++) {
    if (isdigit(string[i]) != 0) {
      return false;
    }
  }

  return true;
}

bool validNumber(char *number) {
  for (int i = 0; i < strlen(number); i++) {
    if (isdigit(number[i]) == 0) {
      return false;
    }
  }

  return true;
}

bool validNamePass(char *name, char *pass) {
  if (strlen(name) == 0 || strlen(pass) == 0) {
    return false;
  }

  if (name[0] == '\n' || pass[0] == '\n') {
    return false;
  }

  for (int i = 0; i < strlen(name); i++) {
    if (name[i] == ' ') {
      return false;
    }
  }

  for (int i = 0; i < strlen(pass); i++) {
    if (pass[i] == ' ') {
      return false;
    }
  }

  return true;
}

int main(int argc, char *argv[]) {
  bool log_acc = 0;
  bool lib_acc = 0;
  char cookie[10000];
  char token[10000];

  char *message;
  char *response;
  int sockfd;

  while (1) {
    char command[50];

    // Citire comanda
    fgets(command, 50, stdin);

    if (strcmp(command, "exit\n") == 0) {  // Exit
      // Rsetare variabile pentru a nu ramane in memorie
      log_acc = 0;
      lib_acc = 0;
      memset(cookie, '\0', sizeof(cookie));
      memset(token, '\0', sizeof(token));
      break;
    }

    if (strcmp(command, "register\n") == 0) {
      char username[100];  // Citire username si parola
      char password[100];
      printf("username=");
      fgets(username, 100, stdin);
      printf("password=");
      fgets(password, 100, stdin);
      username[strlen(username) - 1] = '\0';
      password[strlen(password) - 1] = '\0';

      if (validNamePass(username, password) ==
          false) {  // Verificare username si parola sa nu contina spatii sau sa
                    // fie goale
        printf("EROARE: Nume sau parola gresite\n");
        continue;
      }

      JSON_Value *root_value = json_value_init_object();  // Creez JSON
      JSON_Object *root_object = json_value_get_object(root_value);
      json_object_set_string(root_object, "username", username);
      json_object_set_string(root_object, "password", password);

      char *data = json_serialize_to_string_pretty(root_value);

      // Trimitere request
      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_post_request(HOST, "/api/v1/tema/auth/register",
                                     "application/json", data, NULL, NULL);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      // Interpretez raspunsul
      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE: Nume deja folosit\n");
      } else {
        printf("SUCCES: Cont creat cu succes\n");
      }

      // Eliberez memoria si inchid conexiunea
      json_free_serialized_string(data);
      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }

    if (strcmp(command, "login\n") == 0) {
      // Verific daca sunt deja logat
      if (log_acc == 1) {
        printf("EROARE: Sunteti deja logat\n");
        continue;
      }

      // Citire username si parola
      char username[100];
      char password[100];
      printf("username=");
      fgets(username, 100, stdin);
      username[strlen(username) - 1] = '\0';
      printf("password=");
      fgets(password, 100, stdin);
      password[strlen(password) - 1] = '\0';

      // Verificare username si parola sa nu contina spatii sau sa fie goale
      if (validNamePass(username, password) == false) {
        printf("EROARE: Nume sau parola gresite\n");
        continue;
      }

      // Creez JSON
      JSON_Value *root_value = json_value_init_object();
      JSON_Object *root_object = json_value_get_object(root_value);
      json_object_set_string(root_object, "username", username);
      json_object_set_string(root_object, "password", password);

      char *data = json_serialize_to_string_pretty(root_value);

      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_post_request(HOST, "/api/v1/tema/auth/login",
                                     "application/json", data, NULL, NULL);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      //  Interpretez raspunsul
      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE: Nume sau parola gresite\n");

      } else {
        // Salvez cookie-ul
        char *start = strstr(response, "connect");
        char *end = strstr(response, "; Path");
        memcpy(cookie, start, end - start);
        log_acc = 1;
        printf("SUCCES: Logare cu succes\n");
      }

      json_free_serialized_string(data);
      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }

    if (strcmp(command, "enter_library\n") == 0) {
      // Verific daca sunt logat
      if (log_acc == 0) {
        printf("EROARE: Trebuie sa fiti logat\n");
        continue;
      }

      // Verific daca sunt deja in biblioteca
      if (lib_acc == 1) {
        printf("EROARE: Sunteti deja in biblioteca\n");
        continue;
      }

      // Trimit request
      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL,
                                    cookie, NULL);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      // Interpretez raspunsul
      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE: Nu aveti acces la biblioteca\n");
      } else {
        char *start = strstr(response, "{\"token\":\"");
        start = start + strlen("{\"token\":\"");
        strcpy(token, start);

        int i = 0;
        while (token[i] != '\"' && token[i] != '\0') i++;
        token[i] = '\0';

        if (token[0] != '\0') {
          lib_acc = 1;
          printf("SUCCES: Acces la biblioteca\n");
        }
      }

      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }

    if (strcmp(command, "get_books\n") == 0) {
      // Verific daca sunt logat si daca am acces la biblioteca
      if (log_acc == 0) {
        printf("EROARE: Trebuie sa fiti logat\n");
        continue;
      }

      if (lib_acc == 0) {
        printf("EROARE: Trebuie sa aveti acces la biblioteca\n");
        continue;
      }

      // Trimit request
      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

      message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL,
                                    cookie, token);

      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE\n");
      } else {
        if (strstr(response, "200 OK") != NULL) {
          char *booksExtracted = strstr(response, "[{\"id\":");

          if (booksExtracted == NULL) {
            printf("EROARE: Nu exista carti in biblioteca\n");
          } else {
            printf("%s\n", booksExtracted);
          }
        }
      }

      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }

    if (strcmp(command, "get_book\n") == 0) {
      // Verific daca sunt logat si daca am acces la biblioteca
      if (log_acc == 0) {
        printf("EROARE: Trebuie sa fiti logat\n");
        continue;
      }

      if (lib_acc == 0) {
        printf("EROARE: Trebuie sa aveti acces la biblioteca\n");
        continue;
      }

      // Citire id
      char id[1000];
      printf("id=");
      fgets(id, 1000, stdin);
      id[strlen(id) - 1] = '\0';

      // Verificare id sa nu fie gol sau sa contina spatii sau litere
      if (id[0] == '\n') {
        printf("EROARE: ID invalid\n");
        continue;
      }

      if (validNumber(id) == false) {
        printf("EROARE: ID invalid\n");
        continue;
      }

      // Construire url
      char url[1000] = "/api/v1/tema/library/books/";
      strcat(url, id);

      // Trimit request
      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_get_request(HOST, url, NULL, cookie, token);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      // Interpretez raspunsul

      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE:Nu exista cartea cu id-ul: %s!\n", id);
      } else if (strstr(response, "404 Not Found") != NULL) {
        printf("EROARE:Nu exista cartea cu id-ul: %s!\n", id);
      } else {
        char *book = strstr(response, "{\"id\":");
        if (book == NULL) {
          printf("EROARE:Nu exista cartea cu id-ul: %s!\n", id);
        } else {
          printf("%s\n", book);
        }
      }

      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }

    if (strcmp(command, "add_book\n") == 0) {
      // Verific daca sunt logat si daca am acces la biblioteca
      if (log_acc == 0) {
        printf("EROARE: Trebuie sa fiti logat\n");
        continue;
      }

      if (lib_acc == 0) {
        printf("EROARE: Trebuie sa aveti acces la biblioteca\n");
        continue;
      }

      // Citire date
      char title[1000];
      char author[1000];
      char genre[1000];
      char publisher[1000];
      char page_count[1000];

      printf("title=");
      fgets(title, 1000, stdin);
      title[strlen(title) - 1] = '\0';

      printf("author=");
      fgets(author, 1000, stdin);
      author[strlen(author) - 1] = '\0';

      printf("genre=");
      fgets(genre, 1000, stdin);
      genre[strlen(genre) - 1] = '\0';

      printf("publisher=");
      fgets(publisher, 1000, stdin);
      publisher[strlen(publisher) - 1] = '\0';

      printf("page_count=");
      fgets(page_count, 1000, stdin);
      page_count[strlen(page_count) - 1] = '\0';

      if (title[0] == '\n' || author[0] == '\n' || genre[0] == '\n' ||
          publisher[0] == '\n' || page_count[0] == '\n') {
        printf("EROARE: Date invalide\n");
        continue;
      }

      if (title[0] == '\0' || author[0] == '\0' || genre[0] == '\0' ||
          publisher[0] == '\0' || page_count[0] == '\0') {
        printf("EROARE: Date invalide\n");
        continue;
      }

      // Verificare numar de pagini sa fie valid
      if (validNumber(page_count) == false) {
        printf("EROARE: Numar de pagini invalid\n");
        continue;
      }

      // Creez JSON
      JSON_Value *root_value = json_value_init_object();
      JSON_Object *root_object = json_value_get_object(root_value);
      json_object_set_string(root_object, "title", title);
      json_object_set_string(root_object, "author", author);
      json_object_set_string(root_object, "genre", genre);
      json_object_set_string(root_object, "publisher", publisher);
      json_object_set_number(root_object, "page_count", atoi(page_count));

      char *data = json_serialize_to_string_pretty(root_value);

      // Trimit request
      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_post_request(HOST, "/api/v1/tema/library/books",
                                     "application/json", data, cookie, token);

      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      // Interpretez raspunsul
      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE: Date invalide\n");
      } else {
        printf("SUCCES: Carte adaugata cu succes\n");
      }

      // Eliberez memoria si inchid conexiunea
      json_free_serialized_string(data);
      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }

    if (strcmp(command, "delete_book\n") == 0) {
      // Verific daca sunt logat si daca am acces la biblioteca
      if (log_acc == 0) {
        printf("EROARE: Trebuie sa fiti logat\n");
        continue;
      }

      if (lib_acc == 0) {
        printf("EROARE: Trebuie sa aveti acces la biblioteca\n");
        continue;
      }

      // Citire id si verificare sa fie valid
      char id[1000];
      printf("id=");
      fgets(id, 1000, stdin);
      id[strlen(id) - 1] = '\0';

      if (id[0] == '\n') {
        printf("EROARE: ID invalid\n");
        continue;
      }

      if (validNumber(id) == false) {
        printf("EROARE: ID invalid\n");
        continue;
      }

      // Construire url
      char url[1000] = "/api/v1/tema/library/books/";
      strcat(url, id);

      // Trimit request
      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_delete_request(HOST, url, cookie, token);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      // Interpretez raspunsul
      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE: ID invalid\n");
      } else if (strstr(response, "404 Not Found") != NULL) {
        printf("EROARE: Cartea nu este in biblioteca\n");
      } else {
        printf("SUCCES: Carte stearsa cu succes\n");
      }

      // Eliberez memoria si inchid conexiunea
      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }
    if (strcmp(command, "logout\n") == 0) {
      // Verific daca sunt logat
      if (log_acc == 0) {
        printf("EROARE: Trebuie sa fiti logat\n");
        continue;
      }

      // Trimit request
      sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

      message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL,
                                    cookie, NULL);
      send_to_server(sockfd, message);
      response = receive_from_server(sockfd);

      // Interpretez raspunsul
      if (strstr(response, "error") != NULL ||
          strstr(response, "Bad Request") != NULL) {
        printf("EROARE: Eroare la delogare\n");
      } else {
        //  Delogare cu succes si resetare variabile
        printf("SUCCES: Delogare cu succes\n");
        log_acc = 0;
        lib_acc = 0;
        memset(cookie, '\0', sizeof(cookie));
        memset(token, '\0', sizeof(token));
      }

      close_connection(sockfd);
      message = NULL;
      response = NULL;
    }
  }

  return 0;
}
