CREATE TABLE tcot_bovespa (
    dt_pregao DATE NOT NULL,
    prz_termo INTEGER NOT NULL,
    cd_codneg VARCHAR(23) NOT NULL,
    cd_tpmerc INTEGER NOT NULL,
    cd_codbdi INTEGER NOT NULL,
    cd_codisin VARCHAR(26) NOT NULL,
    nm_speci VARCHAR(11) NOT NULL,
    prec_aber DOUBLE PRECISION NOT NULL,
    prec_max DOUBLE PRECISION NOT NULL,
    prec_min DOUBLE PRECISION NOT NULL,
    prec_med DOUBLE PRECISION NOT NULL,
    prec_fec DOUBLE PRECISION NOT NULL,
    prec_exer DOUBLE PRECISION NOT NULL,
    dt_datven DATE NOT NULL,
    fat_cot INTEGER NOT NULL,
    nr_dismes INTEGER NOT NULL,
    PRIMARY KEY (dt_pregao, prz_termo, cd_codneg)
);
