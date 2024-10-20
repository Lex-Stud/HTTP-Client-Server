Am declarat variabilele:

char command[20]: pentru citirea de la tastura a comenzilor
bool log_acc: pentru a verifica daca utilizatorul este logat
bool lob_acc: pentru a verifca daca am acces in biblioteca
char cookie[10000] pentru a salva cookie urile
char token[10000]   pentru a sala token ul de acces in biblioteca



Intr-un while(1)
Am citit comanda de la tastatura si am verificat daca este una din comenzile disponibile:
    -exit
    -register
    -login
    -logout
    -enter_library
    -get_books
    -get_book
    -add_book
    -delete_book

============================================================================================

register:
    -am deschis conexiunea cu serverul
    -am citit username si password de la tastatura
    -am verificat corectitudinea datelro introduse (Sa nu contina spatii ' ')
    -am facut un request de tip POST la adresa /api/v1/tema/auth/register
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am inchis conexiunea

============================================================================================

login:
    -am deschis conexiunea cu serverul
    -am citit username si password de la tastatura
    -am verificat corectitudinea datelro introduse (Sa nu contina spatii ' ')
    -am facut un request de tip POST la adresa /api/v1/tema/auth/login
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am setat variabila loggedin pe true
    -am salvat cookie urile primit de la server
    -am inchis conexiunea

============================================================================================

logout:
    -am deschis conexiunea cu serverul
    -am facut un request de tip GET la adresa /api/v1/tema/auth/logout
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am setat variabila loggedin pe false
    -am inchis conexiunea

============================================================================================

enter_library:
    -am deschis conexiunea cu serverul
    -am facut un request de tip GET la adresa /api/v1/tema/library/access
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am setat variabila lib_access pe true
    -am salvat tokenul primit de la server
    -am inchis conexiunea

============================================================================================

get_book:
    -am deschis conexiunea cu serverul
    -am citit id-ul cartii de la tastatura
    -am facut un request de tip GET la adresa /api/v1/tema/library/books/id
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am inchis conexiunea

============================================================================================

get_books:
    -am deschis conexiunea cu serverul
    -am facut un request de tip GET la adresa /api/v1/tema/library/books
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am inchis conexiunea

============================================================================================

add_book:
    -am deschis conexiunea cu serverul
    -am citit title, author, genre si publisher de la tastatura
    -am asigurat corectitudinea datelor
    -am facut un request de tip POST la adresa /api/v1/tema/library/books
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am inchis conexiunea

============================================================================================

delete_book:
    -am deschis conexiunea cu serverul
    -am citit id-ul cartii de la tastatura
    -am facut un request de tip DELETE la adresa /api/v1/tema/library/books/id
    -am interpretat raspunsul primit de la server si am afisat un measj corespunzator
    -am inchis conexiunea

============================================================================================

exit:
    -am resetat datele
    -am iesit din program.

============================================================================================

Creare pachet de tip Json
Functii: register login add_book

    -am inițializarea Obiectului JSON:
            JSON_Value *root_value = json_value_init_object();
            JSON_Object *root_object = json_value_get_object(root_value);
    -am setat datele: nume, parla, titlu, autor (dupa caz) etc.
    -am setat datele sub forma de string
            char *data = json_serialize_to_string_pretty(root_value);

============================================================================================

Trimitere HTTP POST:
    -am deschis conexiunea cu serverul
        open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    -am trimis deatele in format Json
        compute_post_request(HOST, url, "application/json", data, cookie, token);
    -am inchis conexiunea

============================================================================================

Fișiere Utilizate
Fișierele helpers.c și .h, requests.c (am adăugat headerele necesare pentru token și cookies) și .h, buffer.c au fost preluate dintr-un laborator pus la dispoziție de laborant.

Alegerea de a folosi parson.c în acest proiect a fost determinată de nevoia de a gestiona eficient și eficace datele JSON într-un mod care să fie atât simplu, cât și performant.
