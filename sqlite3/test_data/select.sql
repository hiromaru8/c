SELECT
    id_src,
    id_dest,
    hex(data) AS data_hex,
    hex(crc)  AS crc_hex
FROM users;

SELECT hex(data)
FROM users
WHERE id_src = 1 AND id_dest = 2;
