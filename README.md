Nume: Vasilescu Andrei
Grupa: 343C3

- In stub-ul server am inclus fisierul "database_management.h" pentru a avea acces la functia care citeste informatiile de input din fisiere pentru a initializa baza de date.
  Functia `load_user_details` va primi ca parametru numele fisierelor date ca argumente la rulare, cat si valabilitatea si le va retine in structuri de date/variabile ce
  constituie baza de date a server-ului.

## Proceduri implementate:

- `REQUEST_AUTHORIZATION` va primi de la client id-ul user-ului ce doreste autentificarea si un flag (0/1) in functie de optiunea clientului
  pentru refresh automat al token-ului de access. Serverul va verifica daca id-ul primit exista in baza de date. Procedura va intoarce o structura
  ce va contine un camp pentru auth token si unul pentru o posibila eroare. Daca user-ul nu este gasit in baza de date, se va pune in campul de eroare
  string-ul `USER_NOT_FOUND`, iar campul pentru token va fi un string gol. In caz contrar, campul pentru token va contine token-ul de autorizare si campul de
  eroare va fi gol.

- `REQUEST_ACCESS_TOKEN` va primi payload-ul pe care il primeste si procedura de auotirizare, cu id si flag-ul de auto refresh, alaturi de o structura ce are
  inauntrul ei token-ul de autorizare primit anterior. Procedura va intoarce o structura ce contine access token (gol in caz de eroare), refresh token (camp gol
  daca nu s-a optat pentru auto refresh sau in caz de eroare), camp de eroare (gol daca accesul este permis) si un camp pentru valabilitatea token-ului (numarul
  de operatii valabile, 0 in caz de eroare). Procedura verifica daca end user-ul a semnat token-ul de autorizare, iar daca nu e semnat va intoarce eroarea `REQUEST_DENIED`.
  Daca token-ul este semnat, se va genera token-ul de acces si daca user-ul a optat pentru refresh automat se va genera si token-ul de refresh. Tokenii, cat si
  valabilitatea vor fi intoarse catre client, de asemenea vor fi salvate in baza de date mapari intre id-ul user-ului si access token, id si refresh token daca
  este cazul, access token si operatii ramase si access token si permisiunile aprobate de end user.

- `VALIDATE_DELEGATED_ACTION` va primi o structura ce contine actiunea, resursa la care se doreste accesul si access token-ul user-ului. Va verifica daca token-ul
  este asociat unui id de utilizator (daca nu se intoarce eroarea `PERMISSION_DENIED`), daca token-ul mai are operatii ramase (daca nu se intoarce eroarea `TOKEN_EXPIRED`),
  daca resursa exista (daca nu se intoarce eroarea `RESOURCE_NOT_FOUND`) si daca token-ul permite operatia dorita (daca nu se intoarce eroarea `OPERATION_NOT_PERMITTED`).
  Daca toate verificarile au fost cu succes se va intoarce `PERMISSION_GRANTED`. De asemenea, daca token-ul exista si nu a expirat se va updata si numarul de operatii ramase.

- `APPROVE_REQUEST_TOKEN` va determina statusul semnarii token-ului de autorizare, primind token-ul in payload. Procedura verifica daca request-ul curent are
  alocate permisiuni, iar daca are va semna token-ul si il va intoarce catre client, retinand si in baza de date ca token-ul respectiv a fost semnat. Am implementat
  procesul de semnare prin adaugarea unui flag in structura de raspuns a procedurii care poate fi 0 sau 1. Astfel daca flag-ul se va intoarce catre client cu valoarea 1,
  acesta va stii ca token-ul este semnat, iar daca se va intoarce cu valoarea 0, nu este semnat.

- `REFRESH_TOKEN`: am adaugat aceasta procedura pentru a separa logica de refresh a token-ului de cea de cerere de acces, dar si pentru a putea trimite un payload
  diferit catre aceasta procedura care sa contina doar refresh token-ul. Procedura aceasta va fi apelata doar de clientii ce au optat pentru refresh automat. Astfel,
  token-ul de refresh este primit, se genereaza noul access token si se updateaza baza de date: noul access token va fi mapat la permisiunile pe care le avea vechiul token,
  iar id-ul user-ului va fi mapat la noul access token, resetandu-se si operatiile ramase la valabilitatea data ca input la program. De asemenea, va fi generat si noul
  refresh token, updatandu-se baza de date si pentru acesta. Apoi, se vor intoarce ambii tokeni generati, impreuna cu valabilitatea.

## Client

Clientul va citi linie cu linie din fisierul de input al clientului, dat ca argument la rulare. La fiecare linie, se verifica daca este o operatie de tip `REQUEST` sau de alt tip.

- Operatie `REQUEST`: clientul va crea payload-ul pentru autorizare cu id-ul user-ului din comanda si valoarea pentru refresh automat si va apela procedura de `REQUEST_AUTHORIZATION`.
  Daca se primeste eroare se afiseaza si se trece mai departe. Daca nu se primeste eroare si se primeste un token de autorizare, se va apela procedura `APPROVE_REQUEST_TOKEN` pentru
  procesul de aprobare/semnare a token-ului primit anterior. Apoi va fi apelata procedura `REQUEST_ACCESS_TOKEN` pentru a obtine token-ul de acces. Daca se va primi o eroare, aceasta va
  fi afisata si se va continua cu urmatoarea comanda, daca se primeste un token de acces, client-ul o sa il mapeze cu id-ul user-ului de la comanda actuala si o sa il retina in propriul
  storage. De asemenea, daca user-ul a optat pentru auto refresh, va fi retinut si refresh token-ul in baza de date, dar si numarul de operatii valabile pentru access token pentru a stii
  cand este momentul pentru un refresh.

- Operatie de alt tip: daca user-ul care da comanda a optat pentru auto refresh, se verifica prima data daca este nevoie pentru un refresh (daca token-ul sau actual de acces mai are sau
  nu operatii valabile; daca nu mai are operatii ramase, se va apela procedura `REFRESH_TOKEN` updatand tokenii pentru id-ul user-ului, in storage-ul clientului). Apoi, se va crea
  payload-ul pentru validarea actiunii, populandu-se cu operatia, resursa si cu token-ul de acces corespunzator id-ului din comanda. Acest payload va fi trimis catre procedura
  `VALIDATE_DELEGATED_ACTION`. Daca token-ul de acces a expirat, clientul va sterge din storage maparea dintre id-ul user-ului si token-ul actual. Daca insa se primeste orice alt raspuns
  decat `PERMISSION_DENIED` si user-ul are auto refresh activat (token-ul lui de access are mapat numarul de operatii ramase, in client) va fi decrementat numarul de operatii ramase
  pentru access token-ul respectiv.

## Server

Pe langa functionalitatile procedurilor descrise mai sus, voi detalia cum manageriaza server-ul baza de date:

- Functia `load_user_details` va citi informatiile din fisierele de input si va retine resursele de pe server, userii, cat si permisiunile corespunzatoare request-urilor de acces, ce vor
  avea asociat un index, pentru a stii in functie de valoarea unui contor din server (ce contorizeaza numarul de request-uri primite) ce permisiuni sa alocam token-ului. Pentru reprezentarea
  permisiunilor pe o resursa am folosit o mapare intre numele resursei si o structura `ResourceRights` ce are un camp corespunzator fiecarui tip de actiune posibila, care poate avea valoarea
  de 0 sau 1 (nu este permis sau este permis). Aceasta functie este apelata la pornirea server-ului din stub-ul server.

- Baza de date a server-ului este generata prin mai multe map-uri sau vectori, dupa cum urmeaza:

  - `resources`: vectorul cu numele resurselor valabile pe server
  - `users`: userii din baza de date a server-ului
  - `request_approvals`: map intre id-ul/contor-ul operatiei de request primita si permisiunile pentru acel request. Este de ajutor apoi pentru crearea map-ului `token_permissions` si asocierea
    corecta intre tokeni si permisiuni
  - `token_permissions`: map intre tokeni de access si permisiunile alocate
  - `auth_tokens`: map intre id-ul user-ului si token-ul de autorizare
  - `access_tokens`: map intre id-ul user-ului si token-ul de acces
  - `refresh_tokens`: map intre refresh token si id-ul user-ului
  - `signed_tokens`: map pentru a retine tokenii semnati
  - `token_operations_remaining`: map intre access token si numarul de operatii ramase pentru acesta

- In fisierul `database_management.cpp` sunt implementate multe functii care fie verifica detalii in baza de date, fie updateaza baza de date prin stergeri sau inserari, aici retinandu-se baza de date din server.
