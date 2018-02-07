CREATE TABLE cachemessages (
    id integer primary key,
    cachemessage blob not null,
    expiry timestamp
)
