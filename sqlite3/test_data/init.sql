
CREATE TABLE tb1 (
    id_src  INTEGER,
    id_dest INTEGER,
    data1   BLOB,
    data2   BLOB,
    PRIMARY KEY (id_src, id_dest)
);

CREATE TABLE tb2 (
    id  INTEGER,
    data1   BLOB,
    data2   BLOB,
    PRIMARY KEY (id)
);

