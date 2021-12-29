-- IDS projekt, část čtvrtá
-- 2020/2021
-- Autoři: Ondřej Ondryáš (xondry02), František Nečas (xnecas27)

------ ODSTRANĚNÍ PŘÍPADNÝCH EXISTUJÍCÍCH OBJEKTŮ ------

DROP TABLE Cafe CASCADE CONSTRAINTS;
DROP TABLE OpeningHours CASCADE CONSTRAINTS;
DROP TABLE SUser CASCADE CONSTRAINTS;
DROP TABLE UserCafeRole CASCADE CONSTRAINTS;
DROP TABLE CafeRating CASCADE CONSTRAINTS;
DROP TABLE CafeReview CASCADE CONSTRAINTS;
DROP TABLE CafeReviewRating CASCADE CONSTRAINTS;
DROP TABLE CoffeeBlend CASCADE CONSTRAINTS;
DROP TABLE CoffeeBean CASCADE CONSTRAINTS;
DROP TABLE DrinkOffer CASCADE CONSTRAINTS;
DROP TABLE CoffeeBlendsInDrinkOffers CASCADE CONSTRAINTS;
DROP TABLE CuppingEvent CASCADE CONSTRAINTS;
DROP TABLE CuppingEventOffer CASCADE CONSTRAINTS;
DROP TABLE EventReservation CASCADE CONSTRAINTS;
DROP TABLE CuppingEventReview CASCADE CONSTRAINTS;
DROP TABLE CuppingEventReviewRating CASCADE CONSTRAINTS;

------ VYTVOŘENÍ TABULEK ------

CREATE TABLE Cafe
(
    id             INT PRIMARY KEY,
    name           NVARCHAR2(128)         NOT NULL,
    address_line   NVARCHAR2(128)         NOT NULL,
    city           NVARCHAR2(128)         NOT NULL,
    website        NVARCHAR2(512)         NULL,
    capacity       INT                    NULL,
    description    NCLOB                  NULL,
    average_rating NUMBER(*, 2) DEFAULT 0 NOT NULL
);

-- změna oproti ERD, transformace atributu opening_hours na slabou entitní množinu ke Cafe,
-- aby atributy byly atomické (1NF)
CREATE TABLE OpeningHours
(
    cafe_id     INT,
    day_of_week INT       NOT NULL CHECK (day_of_week >= 0 AND day_of_week <= 6),
    -- důležitá část z TIMESTAMP je pouze čas, nikoliv datum
    time_from   TIMESTAMP NULL,
    time_to     TIMESTAMP NULL,

    CONSTRAINT PK_OpeningHours PRIMARY KEY (cafe_id, day_of_week, time_from),
    CONSTRAINT FK_OpeningHours_Cafe FOREIGN KEY (cafe_id) REFERENCES Cafe (id) ON DELETE CASCADE,
    -- buď jsou obě pole NULL (= kavárna je v daný den zavřená), nebo musí být _from dříve než _to
    -- porovnáváme ale pouze časy, nemůžeme tedy použít porovnávací operátor nad TIMESTAMP přímo
    CONSTRAINT OpeningHours_Time CHECK
        ((time_from IS NULL AND time_to IS NULL)
            OR
         (time_from IS NOT NULL AND time_to IS NOT NULL AND (EXTRACT(HOUR FROM time_from) < EXTRACT(HOUR FROM time_to)
             OR (EXTRACT(HOUR FROM time_from) = EXTRACT(HOUR FROM time_to) AND
                 EXTRACT(MINUTE FROM time_from) < EXTRACT(MINUTE FROM time_to)))))
);

CREATE TABLE SUser
(
    id                  INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    favourite_drink     NVARCHAR2(128)  NULL,
    favourite_coffee    NVARCHAR2(128)  NULL,
    cups_per_day        INT             NULL,
    profile_description NVARCHAR2(1024) NULL,
    email               VARCHAR2(320)   NOT NULL UNIQUE -- maximální délka per RFC 3696
        -- tento regulární výraz není pro ověřování e-mailových adres zcela ideální, ale pro demonstraci
        -- a většinu reálných e-mailových adres naprosto stačí
        CHECK (REGEXP_LIKE(email, '[a-zA-Z0-9._%-]+@[a-zA-Z0-9._%-]+\.[a-zA-Z]{2,6}')),
    password            VARCHAR2(128)   NOT NULL,       -- změna oproti ERD (+)

    -- vztah "SUser has a favourite Cafe" (M:1)
    favourite_cafe_id   INT             NULL,
    CONSTRAINT FK_SUser_Cafe FOREIGN KEY (favourite_cafe_id) REFERENCES Cafe (id) ON DELETE SET NULL
);

-- "vazební tabulka" pro vztah "SUser has UserCafeRoles in Cafe" (M:M)
CREATE TABLE UserCafeRole
(
    user_id       INT                    NOT NULL,
    cafe_id       INT                    NOT NULL,
    is_owner      NUMBER(1, 0) DEFAULT 0 NOT NULL CHECK (is_owner IN (0, 1)),
    position_name NVARCHAR2(64)          NOT NULL,

    CONSTRAINT PK_UserCafeRole PRIMARY KEY (user_id, cafe_id),
    CONSTRAINT FK_UserCafeRole_User FOREIGN KEY (user_id) REFERENCES SUser (id) ON DELETE CASCADE,
    CONSTRAINT FK_UserCafeRole_Cafe FOREIGN KEY (cafe_id) REFERENCES Cafe (id) ON DELETE CASCADE
);

-- "vazební tabulka" pro vztah "SUser gives CafeRatings for Cafes" (M:M)
CREATE TABLE CafeRating
(
    user_id       INT                       NOT NULL,
    cafe_id       INT                       NOT NULL,
    rating        NUMBER(2, 1)              NOT NULL,
    CHECK (rating >= 0.0 AND rating <= 5.0),
    date_modified DATE DEFAULT CURRENT_DATE NOT NULL,

    CONSTRAINT PK_CafeRating PRIMARY KEY (user_id, cafe_id),
    CONSTRAINT FK_CafeRating_User FOREIGN KEY (user_id) REFERENCES SUser (id) ON DELETE CASCADE,
    CONSTRAINT FK_CafeRating_Cafe FOREIGN KEY (cafe_id) REFERENCES Cafe (id) ON DELETE CASCADE
);

CREATE TABLE CafeReview
(
    id              INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    content         NVARCHAR2(2000)           NOT NULL,
    date_added      DATE DEFAULT CURRENT_DATE NOT NULL,
    date_of_visit   DATE                      NULL,
    points_up       INT  DEFAULT 0            NOT NULL,
    points_down     INT  DEFAULT 0            NOT NULL,

    made_by_user_id INT                       NULL REFERENCES SUser (id) ON DELETE SET NULL,      -- změna oproti ERD (nullable)
    reacts_to_id    INT                       NULL REFERENCES CafeReview (id) ON DELETE SET NULL, -- změna oproti ERD (nullable)
    reviews_cafe_id INT                       NOT NULL REFERENCES Cafe (id) ON DELETE CASCADE
);

-- "vazební tabulka" pro vztah "SUser gives ReviewRatings that rate CafeReviews" (M:M)
CREATE TABLE CafeReviewRating
(
    user_id       INT                       NOT NULL,
    review_id     INT                       NOT NULL,
    is_positive   NUMBER(1, 0)              NOT NULL CHECK (is_positive IN (0, 1)),
    date_modified DATE DEFAULT CURRENT_DATE NOT NULL,

    CONSTRAINT PK_CafeReviewRating PRIMARY KEY (user_id, review_id),
    CONSTRAINT FK_CafeReviewRating_User FOREIGN KEY (user_id) REFERENCES SUser (id) ON DELETE CASCADE,
    CONSTRAINT FK_CafeReviewRating_Review FOREIGN KEY (review_id) REFERENCES CafeReview (id) ON DELETE CASCADE
);

CREATE TABLE CoffeeBlend
(
    id                INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name              NVARCHAR2(128)                    NOT NULL,
    description       NVARCHAR2(2000)                   NOT NULL,
    short_description NVARCHAR2(128)                    NULL,
    roasters          NVARCHAR2(128)                    NULL, -- změna oproti ERD (+)
    date_added        DATE         DEFAULT CURRENT_DATE NOT NULL,
    available         NUMBER(1, 0)                      NOT NULL CHECK (available IN (0, 1)),
    visible           NUMBER(1, 0) DEFAULT 1            NOT NULL CHECK (visible IN (0, 1)),

    -- vztah "Cafe offers CoffeeBlend" (1:M)
    cafe_id           INT                               NOT NULL,
    CONSTRAINT FK_CoffeeBlend_Cafe FOREIGN KEY (cafe_id) REFERENCES Cafe (id) ON DELETE CASCADE
);

CREATE TABLE CoffeeBean
(
    id              INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    variety         NVARCHAR2(64) NULL,
    score           NUMERIC(5, 2) NULL,
    acidity         NUMERIC(3)    NULL
        CHECK (acidity >= 0 AND acidity <= 100),
    body            NUMERIC(3)    NULL
        CHECK (body >= 0 AND body <= 100),
    altitude        NUMERIC(4)    NULL,     -- změna oproti ERD (+)
    roast_type      NVARCHAR2(32) NULL,
    processing_type NVARCHAR2(32) NOT NULL,
    area_of_origin  NVARCHAR2(32) NOT NULL,
    -- změna oproti ERD (- short_description, - description)

    -- vztah "CoffeeBlend consists of CoffeeBean" (1:M)
    blend_id        INT           NOT NULL, -- změna oproti ERD (ze vztahu M:M na vztah 1:M (Blend:Beans))
    CONSTRAINT FK_CoffeeBean_CoffeeBlend FOREIGN KEY (blend_id) REFERENCES CoffeeBlend (id) ON DELETE CASCADE
);

CREATE TABLE DrinkOffer
(
    id          INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name        NVARCHAR2(128)  NOT NULL,
    description NVARCHAR2(1024) NULL,
    price       NUMERIC(5, 2)   NOT NULL,

    -- vztah "Cafe offers DrinkOffer" (1:M)
    cafe_id     INT             NOT NULL,
    CONSTRAINT FK_DrinkOffer_Cafe FOREIGN KEY (cafe_id) REFERENCES Cafe (id) ON DELETE CASCADE
);

-- vazební tabulka pro vztah "DrinkOffer can be made CoffeeBlend" (M:M)
CREATE TABLE CoffeeBlendsInDrinkOffers
(
    coffee_blend_id INT NOT NULL,
    drink_offer_id  INT NOT NULL,

    CONSTRAINT PK_CoffeeBlendsInDrinkOffers PRIMARY KEY (coffee_blend_id, drink_offer_id),
    CONSTRAINT FK_CoffeeBlendsInDrinkOffers_CoffeeBlend
        FOREIGN KEY (coffee_blend_id) REFERENCES CoffeeBlend (id) ON DELETE CASCADE,
    CONSTRAINT FK_CoffeeBlendsInDrinkOffers_DrinkOffer
        FOREIGN KEY (drink_offer_id) REFERENCES DrinkOffer (id) ON DELETE CASCADE
);

CREATE TABLE CuppingEvent
(
    id              INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    name            NVARCHAR2(128)            NOT NULL,
    description     NVARCHAR2(2000)           NOT NULL,
    date_added      DATE DEFAULT CURRENT_DATE NOT NULL,
    event_date      DATE                      NOT NULL,
    total_seats     INT                       NOT NULL
        CHECK (total_seats > 0),
    price           NUMERIC(6, 2)             NOT NULL,

    -- vztah "Cafe organizes CuppingEvents" (1:M)
    cafe_id         INT                       NOT NULL,
    CONSTRAINT FK_CuppingEvent_Cafe FOREIGN KEY (cafe_id) REFERENCES Cafe (id) ON DELETE CASCADE,

    -- vztah "SUser creates CuppingEvents" (1:M)
    -- při případném odstranění uživatele nechceme odstranit všechny informace o proběhlém cuppingu
    made_by_user_id INT                       NULL,
    CONSTRAINT FK_CuppingEvent_SUser FOREIGN KEY (made_by_user_id) REFERENCES SUser (id) ON DELETE SET NULL
);

-- vazební tabulka pro vztah "CuppingEvent offers drinks made from CoffeeBlends" (M:M)
CREATE TABLE CuppingEventOffer
(
    event_id        INT             NOT NULL,
    blend_id        INT             NOT NULL,
    additional_info NVARCHAR2(1024) NULL,

    CONSTRAINT PK_CuppingEventOffer PRIMARY KEY (event_id, blend_id),
    CONSTRAINT FK_CuppingEventOffer_CuppingEvent FOREIGN KEY (event_id) REFERENCES CuppingEvent (id) ON DELETE CASCADE,
    CONSTRAINT FK_CuppingEventOffer_CoffeeBlend FOREIGN KEY (blend_id) REFERENCES CoffeeBlend (id) ON DELETE CASCADE
);

CREATE TABLE EventReservation
(
    email                VARCHAR2(320)             NOT NULL
        CHECK (REGEXP_LIKE(email, '[a-zA-Z0-9._%-]+@[a-zA-Z0-9._%-]+\.[a-zA-Z]{2,4}')),
    event_id             INT                       NOT NULL,
    phone                NUMERIC(16)               NOT NULL,
    date_created         DATE DEFAULT CURRENT_DATE NOT NULL,
    number_of_seats      INT  DEFAULT 1            NOT NULL
        CHECK (number_of_seats > 0),
    date_confirmed       DATE                      NULL,

    CONSTRAINT PK_EventReservation PRIMARY KEY (email, event_id),
    -- vztah "EventReservation reserves seat at CuppingEvent" (M:1)
    CONSTRAINT FK_EventReservation_CuppingEvent FOREIGN KEY (event_id) REFERENCES CuppingEvent (id) ON DELETE CASCADE,
    -- vztah "User makes an EventReservation" (1:M)
    made_by_user_id      INT                       NULL,
    CONSTRAINT FK_EventReservation_MadeBy_SUser FOREIGN KEY (made_by_user_id) REFERENCES SUser (id) ON DELETE SET NULL,
    -- vztah "User confirms an EventReservation" (1:M)
    confirmed_by_user_id INT                       NULL,
    CONSTRAINT FK_EventReservation_ConfirmedBy_SUser FOREIGN KEY (confirmed_by_user_id) REFERENCES SUser (id) ON DELETE SET NULL
);

CREATE TABLE CuppingEventReview
(
    id               INT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    content          NVARCHAR2(2000)           NOT NULL,
    date_added       DATE DEFAULT CURRENT_DATE NOT NULL,
    points_up        INT  DEFAULT 0            NOT NULL,
    points_down      INT  DEFAULT 0            NOT NULL,

    made_by_user_id  INT                       NULL REFERENCES SUser (id) ON DELETE SET NULL,              -- změna oproti ERD (nullable)
    reacts_to_id     INT                       NULL REFERENCES CuppingEventReview (id) ON DELETE SET NULL, -- změna oproti ERD (nullable)
    reviews_event_id INT                       NOT NULL REFERENCES CuppingEvent (id) ON DELETE CASCADE
);

-- "vazební tabulka" pro vztah "SUser gives CuppingEventReviewRatings that rate CuppingEventReviews" (M:M)
CREATE TABLE CuppingEventReviewRating
(
    user_id       INT                       NOT NULL,
    review_id     INT                       NOT NULL,
    is_positive   NUMBER(1, 0)              NOT NULL CHECK (is_positive IN (0, 1)),
    date_modified DATE DEFAULT CURRENT_DATE NOT NULL,

    CONSTRAINT PK_CuppingEventReviewRating PRIMARY KEY (user_id, review_id),
    CONSTRAINT FK_CuppingEventReviewRating_User FOREIGN KEY (user_id) REFERENCES SUser (id) ON DELETE CASCADE,
    CONSTRAINT FK_CuppingEventReviewRating_Review FOREIGN KEY (review_id) REFERENCES CuppingEventReview (id) ON DELETE CASCADE
);

------ POHLEDY ------

DROP MATERIALIZED VIEW UpcomingCuppingEvent;
-- Materializovaný pohled pro získání ještě neproběhlých cuppingových akcí
CREATE MATERIALIZED VIEW UpcomingCuppingEvent
            (Cafe_ID, Cafe_Name, City, Rating, Event_Name, Event_Date, Description, Seats_Left, Seats_Total, Price)
    REFRESH ON DEMAND START WITH (SYSDATE) NEXT (SYSDATE + 1) WITH ROWID
AS
WITH seats_taken AS (
    SELECT EVENT_ID, SUM(NUMBER_OF_SEATS) TAKEN
    FROM EVENTRESERVATION
    GROUP BY EVENT_ID
)
SELECT E.CAFE_ID,
       C.NAME,
       C.CITY,
       C.AVERAGE_RATING,
       E.NAME,
       E.EVENT_DATE,
       E.DESCRIPTION,
       COALESCE(E.TOTAL_SEATS - ST.TAKEN, E.TOTAL_SEATS),
       E.TOTAL_SEATS,
       E.PRICE
FROM CUPPINGEVENT E
         JOIN CAFE C ON C.ID = E.CAFE_ID
         LEFT JOIN seats_taken ST ON E.ID = ST.EVENT_ID
WHERE EVENT_DATE > CURRENT_DATE
ORDER BY EVENT_DATE DESC;

/*
BEGIN
    SYS.DBMS_SCHEDULER.CREATE_JOB
        (job_name => 'UPCOMING_EVENTS_REFRESH',
         start_date => CURRENT_DATE,
         repeat_interval => 'FREQ=DAILY;BYHOUR=0;',
         end_date => NULL,
         job_class => 'DEFAULT_JOB_CLASS',
         job_type => 'PLSQL_BLOCK',
         job_action => 'BEGIN
                            DBMS_MVIEW.REFRESH(''UpcomingCuppingEvent'',''?'');
                        END;',
         comments => 'Job to refresh materialized view UpcomingCuppingEvent.'
        );
END;
*/

-- Pohled pro vylistování uživatelů bez hesel
CREATE OR REPLACE VIEW SUserLimited AS
SELECT ID, favourite_drink, favourite_coffee, cups_per_day, profile_description, email, favourite_cafe_id
FROM SUser;

-- Pohled pro vylistování majitelů kaváren
CREATE OR REPLACE VIEW CafeOwner AS
SELECT user_id, U.email, cafe_id, C.name cafe_name
FROM SUser U
         JOIN UserCafeRole R ON U.ID = R.user_id
         JOIN Cafe C ON C.ID = R.cafe_id
WHERE R.is_owner = 1;

------ TRIGGERY ------

-- Trigger pro zajištění identity u primárního klíče v tabulce Cafe
-- Napodobuje direktivu GENERATED ALWAYS AS IDENTITY, ale nezajišťuje, že se ID po smazání nejnovější položky
-- nebudou opakovat.
CREATE OR REPLACE TRIGGER cafe_pk
    BEFORE INSERT
    ON Cafe
    FOR EACH ROW
BEGIN
    SELECT COALESCE(MAX(id) + 1, 0) INTO :NEW.id FROM Cafe;
END;

-- Trigger aktualizující průměrné hodnocení kavárny při modifikacích hodnocení (změně záznamu CafeRating).
-- Značně neefektivní, viz dokumentace.
CREATE OR REPLACE TRIGGER cafe_average
    AFTER UPDATE OR INSERT OR DELETE
    ON CafeRating
BEGIN
    -- noinspection SqlWithoutWhere
    UPDATE Cafe
    SET average_rating = (SELECT COALESCE(AVG(rating), 0) FROM CafeRating WHERE cafe_id = Cafe.id);
END;

-- Triggery aktualizující points_up a points_down u CafeReview na základě změn CafeReviewRating.
-- Značně neefektivní, viz dokumentace.
CREATE OR REPLACE TRIGGER points_insert
    AFTER UPDATE OR INSERT OR DELETE
    ON CafeReviewRating
BEGIN
    -- noinspection SqlWithoutWhere
    UPDATE CafeReview
    SET points_up   = (SELECT COUNT(*) FROM CafeReviewRating WHERE is_positive = 1 AND review_id = CafeReview.id),
        points_down = (SELECT COUNT(*) FROM CafeReviewRating WHERE is_positive = 0 AND review_id = CafeReview.id);
END;

-- Trigger, který slouží pro sekvenční zadávání otevíracích hodin kavárny.
-- Pokud je při vložení nastaven day_of_week na null, tento trigger vkládanému záznamu nastaví další den po tom,
-- který pro danou kavárnu už existuje.
CREATE OR REPLACE TRIGGER opening_hours_sequential
    BEFORE INSERT
    ON OpeningHours
    FOR EACH ROW
BEGIN
    IF :NEW.day_of_week IS NULL THEN
        SELECT MOD(COALESCE(MAX(OpeningHours.day_of_week) + 1, 0), 7)
        INTO :NEW.day_of_week
        FROM OpeningHours
        WHERE OpeningHours.cafe_id = :NEW.cafe_id;
    END IF;
END;

------ ULOŽENÉ PROCEDURY ------

CREATE OR REPLACE PACKAGE CafeUtils AS
    PROCEDURE set_fav_drink(a_user_id NUMBER, a_drink_offer_id NUMBER);
    PROCEDURE set_cafe_review_rating(a_user_id IN NUMBER, a_review_id IN NUMBER,
                                     a_rating_positive IN NUMBER);
    FUNCTION make_reservation(a_event_id IN NUMBER, a_email IN VARCHAR2, a_phone IN NUMBER,
                              a_number_of_seats IN NUMBER) RETURN BOOLEAN;
END;

CREATE OR REPLACE PACKAGE BODY CafeUtils AS
    -- Procedura pro nastavení uživatelova oblíbeného nápoje podle zadané nabídky nějaké kavárny.
    -- Pokud zadaný uživatel neexistuje, nestane se nic (je vypsáno upozornění).
    -- Pokud zadaný nápoj neexistuje, uživateli je jako favourite_coffee nastavena prázdná hodnota (a je vypsáno upozornění).
    PROCEDURE set_fav_drink(a_user_id NUMBER, a_drink_offer_id NUMBER) AS
        v_drink_name DrinkOffer.name%TYPE;
    BEGIN
        SELECT name INTO v_drink_name FROM DrinkOffer WHERE ID = a_drink_offer_id;

        UPDATE SUser SET favourite_drink = v_drink_name WHERE ID = a_user_id;
        IF SQL%ROWCOUNT = 0 THEN
            DBMS_OUTPUT.PUT_LINE('User with ID ' || a_user_id || ' does not exist.');
        END IF;
    EXCEPTION
        WHEN NO_DATA_FOUND THEN
            UPDATE SUser SET favourite_drink = NULL WHERE ID = a_user_id;
            DBMS_OUTPUT.PUT_LINE('DrinkOffer with ID ' || a_drink_offer_id || ' does not exist.');
    END;

    -- Procedura pro přidání nebo aktualizaci hodnocení recenze kavárny.
    -- Obdobnou by bylo vhodné vytvořit pro CuppingEventReview(Rating).
    -- Pokud zadaný uživatel nebo zadaná recenze neexistují, nestane se nic (je vypsáno upozornění).
    PROCEDURE set_cafe_review_rating(a_user_id IN NUMBER, a_review_id IN NUMBER,
                                     a_rating_positive IN NUMBER) AS
        v_serialization_complete BOOLEAN := FALSE;

        e_invalid_bool EXCEPTION;
        e_serializable EXCEPTION;
        e_integrity EXCEPTION;

        PRAGMA AUTONOMOUS_TRANSACTION; -- Transakce v této proceduře nebudou ovlivňovat průběh jiných transakcí
        PRAGMA EXCEPTION_INIT (e_serializable, -08177); -- Chyba "Can't Serialize Access For This Transaction"
        PRAGMA EXCEPTION_INIT (e_integrity, -02291); -- Chyba "Integrity constraint violated: Parent key not found"
    BEGIN
        -- Pokud není a_rating_positive hodnota 0 nebo 1, vyvoláme uživateli výjimku
        IF a_rating_positive <> 0 AND a_rating_positive <> 1 THEN
            RAISE e_invalid_bool;
        END IF;

        -- Synchronizace: pokoušíme se aktualizovat nebo vložit hodnotu, dokud nám to SŘBD nedovolí
        -- https://stackoverflow.com/a/19929732
        <<SER_LOOP>>
        WHILE NOT v_serialization_complete
            LOOP
                BEGIN
                    SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;

                    -- Aktualizace hodnoty nebo vložení nové
                    MERGE INTO CafeReviewRating
                    USING DUAL
                    ON (USER_ID = a_user_id AND REVIEW_ID = a_review_id)
                    WHEN MATCHED THEN
                        UPDATE
                        SET DATE_MODIFIED = (CASE -- Datum se upraví jen tehdy, když se hodnocení změnilo
                                                 WHEN IS_POSITIVE = a_rating_positive THEN DATE_MODIFIED
                                                 ELSE CURRENT_DATE END),
                            IS_POSITIVE   = a_rating_positive
                    WHEN NOT MATCHED THEN
                        INSERT VALUES (a_user_id, a_review_id, a_rating_positive, CURRENT_DATE);

                    COMMIT;
                    v_serialization_complete := TRUE; -- Ukončí SER_LOOP
                EXCEPTION
                    -- Pokud nastane chyba serializace, transakci vrátíme a pokusíme se provést příkazy znovu
                    WHEN e_serializable THEN
                        ROLLBACK;
                        DBMS_OUTPUT.PUT_LINE('Serialization error, trying again.');
                        CONTINUE SER_LOOP;
                    WHEN e_integrity THEN
                        ROLLBACK;
                        DBMS_OUTPUT.PUT_LINE(
                                    'Integrity error: Either user ' || a_user_id || ' or review ' || a_review_id ||
                                    ' does not exist.');
                        EXIT SER_LOOP;
                END;
            END LOOP; -- SER_LOOP
    END;

    /** Funkce pro vytvoření nebo úpravu rezervace na cupping akci. Zkontroluje počet dostupných míst na akci.
      Klíčem rezervace je ID akce a e-mail, který nemusí příslušet registrovanému uživateli.
      Pokud ovšem uživatel s tímto e-mailem existuje, rezervace je mu automaticky přiřazena.
      Funkce vrací TRUE, pokud bylo vytvoření rezervace úspěšné – na akci zbývalo minimálně tolik míst, kolik
      bylo vyžadováno. Vrací FALSE, pokud akce s daným ID neexistuje nebo pokud na akci nezbývalo dost míst.
      Pokud je počet míst 0, případná stávající rezervace je odstraněna.
      Pokud už existuje rezervace pod daným e-mailem pro danou akci a funkce je zavolána s jiným požadovaným počtem míst,
      než je uveden v existující rezervaci, existující rezervace je vrácena do stavu „nově vytvořené“ – údaje o jejím
      potvrzení jsou NULL. Pokud je uveden stejný počet míst jako v existující rezervaci, je změněn pouze telefon, případně
      vazba na uživatele.*/
    FUNCTION make_reservation(a_event_id IN NUMBER, a_email IN VARCHAR2, a_phone IN NUMBER,
                              a_number_of_seats IN NUMBER) RETURN BOOLEAN AS
        e_invalid_bool EXCEPTION;
        e_serializable EXCEPTION;

        v_user_id     SUSER.ID%TYPE;
        v_resv_seats  EVENTRESERVATION.NUMBER_OF_SEATS%TYPE;
        v_seats_left  CUPPINGEVENT.TOTAL_SEATS%TYPE;
        v_seats_taken EVENTRESERVATION.NUMBER_OF_SEATS%TYPE;
        v_rc          INTEGER;

        PRAGMA AUTONOMOUS_TRANSACTION; -- Transakce v této proceduře nebudou ovlivňovat průběh jiných transakcí
        PRAGMA EXCEPTION_INIT (e_serializable, -08177); -- Chyba "Can't Serialize Access For This Transaction"

        -- Pro demonstraci použití kurzoru jej využijeme k odečtení už rezervovaných míst
        -- (podle tabulky EventReservations) z celkového počtu míst (Event.total_seats).
        -- Totéž by šlo provést i s využitím agregační funkce.
        CURSOR c_reservations IS
            SELECT NUMBER_OF_SEATS
            FROM EVENTRESERVATION
            WHERE EVENT_ID = a_event_id
              AND EMAIL <> a_email;
    BEGIN
        -- Synchronizace: pokoušíme se aktualizovat nebo vložit hodnotu, dokud nám to SŘBD nedovolí
        -- https://stackoverflow.com/a/19929732
        <<SER_LOOP>>
        WHILE TRUE
            LOOP
                BEGIN
                    SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;

                    IF a_number_of_seats = 0 THEN
                        DELETE FROM EventReservation WHERE event_id = a_event_id AND email = a_email;
                        v_rc := SQL%ROWCOUNT;
                        COMMIT;
                        IF v_rc > 0 THEN -- Pokud byl smazán nějaký řádek, aktualizujme pohled
                            DBMS_SNAPSHOT.REFRESH('UPCOMINGCUPPINGEVENT', '?');
                        END IF;
                        RETURN TRUE;
                    END IF;

                    SELECT total_seats
                    INTO v_seats_left
                    FROM CuppingEvent
                    WHERE id = a_event_id;

                    OPEN c_reservations;
                    LOOP
                        FETCH c_reservations INTO v_seats_taken;
                        EXIT WHEN c_reservations%NOTFOUND;
                        v_seats_left := v_seats_left - v_seats_taken;
                    END LOOP;
                    CLOSE c_reservations;

                    IF a_number_of_seats <= v_seats_left
                    THEN
                        -- Pokusíme se najít uživatele
                        BEGIN
                            SELECT ID
                            INTO v_user_id
                            FROM SUser
                            WHERE EMAIL = a_email;
                        EXCEPTION
                            WHEN NO_DATA_FOUND THEN
                                v_user_id := NULL;
                        END;

                        -- Aktualizace hodnoty nebo vložení nové
                        BEGIN
                            SELECT number_of_seats
                            INTO v_resv_seats
                            FROM EventReservation
                            WHERE event_id = a_event_id
                              AND email = a_email;

                            -- Pokud se změnil počet míst, chovejme se k záznamu jako k nové rezervaci
                            IF v_resv_seats <> a_number_of_seats THEN
                                UPDATE EventReservation
                                SET number_of_seats      = a_number_of_seats,
                                    made_by_user_id      = COALESCE(v_user_id, made_by_user_id),
                                    date_created         = CURRENT_DATE,
                                    date_confirmed       = NULL,
                                    confirmed_by_user_id = NULL,
                                    phone                = a_phone
                                WHERE event_id = a_event_id
                                  AND email = a_email;
                            ELSE -- Jinak pouze proveďme případnou změnu telefonu a přiřazeného uživatele
                                UPDATE EventReservation
                                SET made_by_user_id = COALESCE(v_user_id, made_by_user_id),
                                    phone           = a_phone
                                WHERE event_id = a_event_id
                                  AND email = a_email;
                            END IF;
                        EXCEPTION
                            -- Nastane, pokud v EventReservation neexistuje daná dvojice event_id/email
                            WHEN NO_DATA_FOUND THEN
                                INSERT INTO EventReservation (email, event_id, phone, date_created, number_of_seats,
                                                              date_confirmed, made_by_user_id, confirmed_by_user_id)
                                VALUES (a_email, a_event_id, a_phone, CURRENT_DATE, a_number_of_seats, NULL, v_user_id,
                                        NULL);
                        END;

                        COMMIT;
                        -- https://docs.oracle.com/database/121/DWHSG/refresh.htm#DWHSG9416
                        DBMS_SNAPSHOT.REFRESH('UPCOMINGCUPPINGEVENT', '?');
                        RETURN TRUE;
                    ELSE
                        COMMIT;
                        DBMS_OUTPUT.PUT_LINE('Event has less seats left than required.');
                        RETURN FALSE;
                    END IF;
                EXCEPTION
                    -- Pokud nastane chyba serializace, transakci vrátíme a pokusíme se provést příkazy znovu
                    WHEN
                        e_serializable THEN
                        ROLLBACK;
                        CONTINUE SER_LOOP;
                    WHEN NO_DATA_FOUND THEN
                        ROLLBACK;
                        DBMS_OUTPUT.PUT_LINE('Event does not exist.');
                        RETURN FALSE;
                END;
            END LOOP; -- SER_LOOP
    END;
END;

------ PŘÍSTUPOVÁ PRÁVA PRO DRUHÉHO ČLENA TÝMU ------
GRANT SELECT, INSERT, UPDATE ON Cafe TO xnecas27;
GRANT SELECT, INSERT, UPDATE ON OpeningHours TO xnecas27;
GRANT SELECT, INSERT ON CafeReview TO xnecas27;
GRANT SELECT, INSERT ON CafeRating TO xnecas27;
GRANT SELECT ON CafeReviewRating TO xnecas27;
GRANT SELECT ON CoffeeBlend TO xnecas27;
GRANT SELECT ON CoffeeBean TO xnecas27;
GRANT SELECT ON CoffeeBlendsInDrinkOffers TO xnecas27;
GRANT SELECT ON DrinkOffer TO xnecas27;
-- Pohledy
GRANT SELECT ON UpcomingCuppingEvent TO xnecas27;
GRANT SELECT, UPDATE ON SUserLimited TO xnecas27;
GRANT SELECT ON CafeOwner TO xnecas27;
-- Balíček procedur
GRANT EXECUTE ON CafeUtils TO xnecas27;
-- Nemá přístup k tabulkám o událostech a o uživatelích
REVOKE ALL PRIVILEGES ON CuppingEvent FROM xnecas27;
REVOKE ALL PRIVILEGES ON CuppingEventOffer FROM xnecas27;
REVOKE ALL PRIVILEGES ON CuppingEventReview FROM xnecas27;
REVOKE ALL PRIVILEGES ON CuppingEventReviewRating FROM xnecas27;
REVOKE ALL PRIVILEGES ON EventReservation FROM xnecas27;
REVOKE ALL PRIVILEGES ON SUser FROM xnecas27;
REVOKE ALL PRIVILEGES ON UserCafeRole FROM xnecas27;

------ TESTOVACÍ DATA ------

INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Café Flexaret', 'Kopečná 2/21', 'Brno', 'https://www.facebook.com/CafeFlexaret', 40, 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean cursus hendrerit cursus. Maecenas pellentesque elementum ornare. Vivamus efficitur, purus sit amet ornare dignissim, mauris quam commodo neque, vel condimentum enim velit eget mi. Aenean faucibus libero nibh, nec ornare tortor ultricies et. Vivamus tincidunt nisl at elit lacinia pretium. Nunc non nisi facilisis, tincidunt libero nec, placerat sapien. Nunc scelerisque semper magna sed ultrices.

Mauris lorem dolor, semper id pellentesque ac, volutpat et velit. Vivamus ornare libero eu ante auctor ultricies. Ut ut augue id est tincidunt vulputate quis quis neque. Sed non arcu vel magna vehicula vestibulum. Morbi vitae dui lorem. Fusce ac dui aliquam, cursus dolor sed, tempus odio. Cras quis pharetra nibh. Integer non sagittis nunc. Morbi eleifend metus a hendrerit accumsan.');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Kavárna Spolek', 'Orlí 22', 'Brno', 'https://www.spolek.net', 60,
        'Aenean egestas eleifend libero aliquet tincidunt. In sed lacinia eros, id vulputate eros. Proin fringilla id lectus sed malesuada. Fusce ut rhoncus enim, in scelerisque dolor. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Praesent ultricies neque at varius porttitor. Etiam eget hendrerit urna, venenatis egestas nunc. Sed sagittis sapien ut magna rutrum blandit. Suspendisse eget placerat ex. Nunc maximus posuere felis eget fringilla. Quisque nec nisi nulla. Vestibulum sit amet orci ac est molestie iaculis at eu tellus. Vivamus semper, ante vitae gravida eleifend, turpis nisl interdum nisi, id ultrices elit purus at ante.');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('The Roses', 'Purkyňova 2415/114', 'Brno', 'https://theroses.cz', 20, 'Donec nec dui sit amet mauris porttitor consequat a efficitur lacus. Fusce id orci non dolor sollicitudin finibus. Donec auctor sem at nulla sollicitudin ultricies. Aenean cursus, nisl ut porta dignissim, mauris erat elementum ligula, vehicula ullamcorper nibh nibh non ex. Phasellus vehicula ipsum sem, ut placerat enim viverra congue. Aliquam non turpis turpis. Nulla lacinia vestibulum neque ac semper. Sed pharetra eu libero nec egestas. Suspendisse sed accumsan felis.

Nunc convallis sollicitudin dictum. Suspendisse potenti. Vestibulum tincidunt elit nunc, non vulputate sapien aliquam dapibus. Morbi varius, ipsum nec volutpat porttitor, lacus dolor scelerisque mi, id ullamcorper leo lectus at metus. Aenean luctus lectus diam, sit amet auctor mi elementum vitae. Mauris a magna non lacus volutpat elementum. Aenean tempor turpis ex, non commodo justo luctus a. Sed pellentesque non massa non tempor. Phasellus non lectus lacinia, ullamcorper lacus nec, commodo tortor. Vivamus sit amet purus facilisis sem efficitur malesuada. Etiam vitae lectus quis velit blandit luctus a non ex. Nunc rutrum ultricies turpis. Quisque eget arcu enim. Integer fermentum accumsan justo id ornare. Cras interdum elit id urna dignissim, vel faucibus metus commodo. Nulla imperdiet vestibulum ultrices.');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Caffe Rotatoria', 'V Újezdech 535', 'Brno-Medlánky', 'https://www.rotatoria.cz', 30,
        'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Mandlárna Brno', 'Dominikánská 352/1', 'Brno', 'https://mandlarna.cz/mandlova-kavarna-mandlarna-brno', 55,
        'Curabitur vehicula ex porta efficitur dapibus. Phasellus auctor, dui eu convallis dictum, ligula diam sodales felis, et suscipit dolor risus et mauris. Donec facilisis ut ligula lobortis scelerisque. Etiam iaculis bibendum pretium. Ut sit amet libero luctus, aliquam nulla et, eleifend nulla. Suspendisse id nulla suscipit arcu finibus tempor tincidunt vel diam. Sed tempus posuere feugiat. Phasellus porttitor feugiat mi, non tempor quam pretium sit amet. Integer varius diam non interdum condimentum.');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Cafe Park Slovaňák', 'Slovanské náměstí 1804/7', 'Brno-Královo Pole', 'generated', 35,
        'Lorem Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia curae; Etiam fringilla a tortor vitae egestas. Quisque a nulla mattis, commodo lectus egestas, imperdiet nunc. Praesent ipsum magna, sagittis sed blandit ut, hendrerit et nisi. Quisque ac ante luctus, vestibulum tortor vel, accumsan arcu. Sed sodales, arcu id maximus porta, nulla nisl imperdiet magna, vel porta diam libero sit amet sem. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Vivamus sollicitudin faucibus arcu sit amet blandit. Sed ullamcorper sit amet augue et condimentum. dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Večerka', 'Pekařská 9', 'Brno', 'https://facebook.com/vecerkabrno', 25,
        'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Rebelbean Vlněna', 'Přízova 5', 'Brno', 'https://facebook.com/rebelbeanvlnena', 50,
        'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Café Mitte', 'Panská 11', 'Brno', 'https://www.cafemitte.com', 45, 'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Zemanova kavárna a cukrárna', 'Josefská 493/4', 'Brno', 'https://www.facebook.com/zemanka', 75,
        'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Jádro Café', 'Purkyňova 35b', 'Brno-Královo Pole', 'https://jadrocafe.cz', 10,
        'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Veroni Coffee & Chocolate', 'Gorkého 85/42', 'Brno', 'https://www.veroni.cz', 35,
        'Když jsme na konci roku 2015 naši kavárnu téměř bez jakýchkoliv zkušeností otevírali, byli jsme velmi troufalí, možná až drzí. Byli jsme však také plní nadšení vrhnout se do toho po hlavě. Vsadili jsme na poctivé dezerty vlastní výroby, kvalitní kávu a nyní i vlastnoručně vyrobenou čokoládu. Postupem času jsme se zdokonalovali a získávali si srdce mnohých z Vás. Toho si moc vážíme, a tak jsme chtěli, abyste se u nás cítili ještě lépe. Kabát kavárny jsme sice změnili, náš úsměv a srdečný přístup ale zůstává :)');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Košilka', 'Nerudova 318/4', 'Brno', 'generated', 3,
        'Naše kavárna je místem pro setkávání, diskuze, učení, oddych, vždy u voňavé kávy a něčeho dobrého k tomu. Chceme, abyste se u nás cítili co nejlépe. Buďte vítáni');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Šestá větev', 'Moravské náměstí 12', 'Brno', 'https://facebook.com/sestavetev', 20,
        'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Coffee Fusion', 'Jánská 460/25', 'Brno', 'generated', 40, 'Lorem ipsum dolor sit amet');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Monogram Espresso Bar', 'Kapucínské náměstí 310/12', 'Brno', 'https://www.monogramespressobar.cz',
        5, 'Ať je pro vás Monogram místem, kde tepe život a vlévá vám do žil nejen kofein, ale i úsměv a radost.');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Balloo Coffe', 'Nádražní 624', 'Rožnov pod Radhoštěm', 'https://balloo.cz', 45,
        'Káva je v naší kavárně priorita číslo jedna. Naše holky, jsou pečlivě proškolené baristickými kurzy,stovkami litrů našlehaného mléka a ostřílenými zakazníky ze všech koutů světa. Měli by jste se sami přesvědčit..a ke káve si můžete dát třeba něco z našich francouzských dezertů.');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Anděl penzion a café', 'Pod strání 580', 'Rožnov pod Radhoštěm', 'generated', 25,
        'PENZION, KAVÁRNA i SAUNA – zavřeno, vydejní okýnko také zatím zavřeno,🎂upečení celého dortu – volejte 775359021,🔑 ubytování pouze pracovní cesty – volejte 602541112😇');
INSERT INTO Cafe (name, address_line, city, website, capacity, description)
VALUES ('Café na cucky', 'Dolní náměstí 23', 'Olomouc', 'http://www.cafenacucky.cz', 15,
        'Naše kavárna je součástí kulturního centra, které vzniklo na Dolním náměstí v roce 2017 s podporou více než sedmi stovek přispěvatelů a přátel našeho divadla. Divadlo na cucky není jen divadlo. Běžně zde potkáte české i zahraniční umělce, můžete navštívit představení z repertoáru domácího souboru i špičkových hostujících scén, připravujeme tvořivé dílny i pravidelné workshopy. Najdete tu také dva výstavní prostory Galerie XY věnované současnému výtvarnému a konceptuálnímu umění. V Café na cucky nabízíme výběrovou kávu z českých pražíren Rebelbean a Candycane coffee. Obecně se v nabídce snažíme využívat regionální produkty a usilujeme o co nejvyšší kvalitu. Naši kuchaři pro Vás s chutí a s láskou každý den připravují ty nejlepší dezerty a originální snídaně a polévky. Budeme rádi, když u nás najdete místo pro příjemný čas strávený s přáteli, s rodinou nebo místo, kde potkáte nové známé.');

INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 0, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI')
FROM Cafe;
INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 1, TO_DATE('8:00', 'HH24:MI'), TO_DATE('16:00', 'HH24:MI')
FROM Cafe;
INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 2, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI')
FROM Cafe;
INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 3, TO_DATE('9:00', 'HH24:MI'), TO_DATE('18:00', 'HH24:MI')
FROM Cafe;
INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 4, TO_DATE('8:00', 'HH24:MI'), TO_DATE('12:00', 'HH24:MI')
FROM Cafe;
INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 4, TO_DATE('13:00', 'HH24:MI'), TO_DATE('18:00', 'HH24:MI')
FROM Cafe;
INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 5, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI')
FROM Cafe
WHERE id IN (2, 3, 5, 7, 11, 13);
INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
SELECT id, 6, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI')
FROM Cafe
WHERE id IN (2, 4, 8, 12);
UPDATE OpeningHours
SET time_to = TO_DATE('19:00', 'HH24:MI')
WHERE cafe_id IN (1, 3, 5, 7, 11);

-- password is 123456789 in SHA512
INSERT INTO SUser (favourite_drink, favourite_coffee, cups_per_day, profile_description, email, password,
                   favourite_cafe_id)
VALUES ('V60', 'Keňa', 2, NULL, 'abc@def.cz',
        'd9e6762dd1c8eaf6d61b3c6192fc408d4d6d5f1176d0c29169bc24e71c3f274ad27fcd5811b313d681f7e55ec02d73d499c95455b6b5bb503acf574fba8ffe85',
        (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1));

INSERT INTO SUser (favourite_drink, favourite_coffee, cups_per_day, profile_description, email, password,
                   favourite_cafe_id)
VALUES ('espresso', 'naturální etiopka', 2, 'asdsalkkdůslaklůdsjkqwjekwqjeklasdjůsajkljasdlk', '123456@google.com',
        'd9e6762dd1c8eaf6d61b3c6192fc408d4d6d5f1176d0c29169bc24e71c3f274ad27fcd5811b313d681f7e55ec02d73d499c95455b6b5bb503acf574fba8ffe85',
        (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1));

INSERT INTO SUser (favourite_drink, favourite_coffee, cups_per_day, profile_description, email, password,
                   favourite_cafe_id)
VALUES (NULL, NULL, 1, NULL, 'test@google.com',
        'd9e6762dd1c8eaf6d61b3c6192fc408d4d6d5f1176d0c29169bc24e71c3f274ad27fcd5811b313d681f7e55ec02d73d499c95455b6b5bb503acf574fba8ffe85',
        (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1));

INSERT INTO SUser (favourite_drink, favourite_coffee, cups_per_day, profile_description, email, password,
                   favourite_cafe_id)
VALUES ('latte macchiato', NULL, NULL, 'rwwerwekljífg2d2f1g2fd31w5e3123s3dfdsf', '567489@seznam.cz',
        'd9e6762dd1c8eaf6d61b3c6192fc408d4d6d5f1176d0c29169bc24e71c3f274ad27fcd5811b313d681f7e55ec02d73d499c95455b6b5bb503acf574fba8ffe85',
        NULL);

INSERT INTO SUser (favourite_drink, favourite_coffee, cups_per_day, profile_description, email, password,
                   favourite_cafe_id)
VALUES ('Cappuccino', 'illy', 3, NULL, 'sadadasd@google.com',
        'd9e6762dd1c8eaf6d61b3c6192fc408d4d6d5f1176d0c29169bc24e71c3f274ad27fcd5811b313d681f7e55ec02d73d499c95455b6b5bb503acf574fba8ffe85',
        (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1));

DECLARE
    out_i INT;
begin
    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Keňa, Etiopie', 'Lorem Ipsum', NULL, 'Square Mile Coffee Roasters', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 95, 70, 56, 2465, 'Medium', 'Promytá metoda', 'Keňa', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 83, 88, 60, 2832, 'Medium', 'Anaerobní fermentace', 'Etiopie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Nicaragua', 'Lorem Ipsum', NULL, 'Dabov', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 80, 100, 84, 2883, 'Dark', 'Washed', 'Nicaragua', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Brazílie, Nicaragua, Panama', 'Lorem Ipsum', NULL, 'Dabov', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 99, 99, 100, 2096, 'Light', 'Washed', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 89, 73, 73, 2216, 'Medium', 'Anaerobní fermentace', 'Nicaragua', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 89, 98, 70, 2570, 'Medium', 'Natural', 'Panama', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Brazílie, Nicaragua, Panama', 'Lorem Ipsum', NULL, 'The Space', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Heirloom', 96, 89, 56, 1647, 'Espresso', 'Honey', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 92, 83, 93, 1449, 'Espresso', 'Washed', 'Nicaragua', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 89, 87, 89, 1208, 'Filter', 'Honey', 'Panama', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Peru, Brazílie, Nicaragua', 'Lorem Ipsum', NULL, 'Naughty Dog', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 87, 53, 51, 2382, 'Filter', 'Anaerobní fermentace', 'Peru', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Heirloom', 81, 67, 66, 1087, 'Espresso', 'Suchá metoda', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 81, 80, 82, 1402, 'Medium-Dark', 'Promytá metoda', 'Nicaragua', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Brazílie', 'Lorem Ipsum', NULL, 'Square Mile Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 99, 94, 51, 1983, 'Dark', 'Anaerobní fermentace', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Rwanda', 'Lorem Ipsum', NULL, 'Doubleshot', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 100, 95, 94, 2813, 'Light', 'Anaerobní fermentace', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Kolumbie, Peru', 'Lorem Ipsum', NULL, 'Doubleshot', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL28', 92, 86, 91, 1298, 'Omni', 'Suchá metoda', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL28', 97, 83, 57, 1632, 'Filter', 'Anaerobní fermentace', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Keňa', 'Lorem Ipsum', NULL, 'Naughty Dog', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 98, 85, 84, 2881, 'Filter', 'Natural', 'Keňa', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie, Peru', 'Lorem Ipsum', NULL, 'Naughty Dog', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Heirloom', 87, 68, 66, 1227, 'Dark', 'Promytá metoda', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 94, 63, 68, 1138, 'Omni', 'Natural', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 83, 83, 53, 2957, 'Filter', 'Anaerobní fermentace', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Kolumbie', 'Lorem Ipsum', NULL, 'Dabov', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 99, 83, 97, 1391, 'Dark', 'Promytá metoda', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Kolumbie, Peru', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 100, 86, 79, 2599, 'Filter', 'Anaerobní fermentace', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 80, 90, 52, 1080, 'Light', 'Natural', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Peru, Brazílie, Nicaragua', 'Lorem Ipsum', NULL, 'QB Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Heirloom', 99, 99, 58, 2583, 'Light', 'Honey', 'Peru', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL28', 87, 74, 94, 1185, 'Medium-Dark', 'Promytá metoda', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 92, 67, 83, 1704, 'Dark', 'Suchá metoda', 'Nicaragua', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Rwanda', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 80, 95, 81, 1627, 'Medium', 'Washed', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Peru, Brazílie, Nicaragua', 'Lorem Ipsum', NULL, 'QB Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 93, 60, 82, 1431, 'Medium', 'Honey', 'Peru', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 96, 96, 62, 2708, 'Light', 'Honey', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 86, 67, 99, 2434, 'Espresso', 'Natural', 'Nicaragua', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Panama', 'Lorem Ipsum', NULL, 'Rebelbean', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 80, 53, 62, 1312, 'Medium', 'Washed', 'Panama', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Etiopie, Rwanda, Kolumbie', 'Lorem Ipsum', NULL, 'QB Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 94, 59, 93, 2664, 'Espresso', 'Anaerobní fermentace', 'Etiopie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 90, 58, 92, 1364, 'Light', 'Anaerobní fermentace', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 99, 52, 72, 1396, 'Light', 'Washed', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Peru', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 83, 82, 86, 1905, 'Light', 'Anaerobní fermentace', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Peru', 'Lorem Ipsum', NULL, 'The Space', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 98, 53, 82, 1579, 'Omni', 'Natural', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Kolumbie, Peru, Brazílie', 'Lorem Ipsum', NULL, 'The Space', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 96, 62, 88, 1800, 'Light', 'Promytá metoda', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 86, 55, 72, 2338, 'Medium-Dark', 'Washed', 'Peru', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 91, 51, 57, 2934, 'Light', 'Honey', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Brazílie', 'Lorem Ipsum', NULL, 'Dabov', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 91, 91, 100, 2107, 'Omni', 'Suchá metoda', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie', 'Lorem Ipsum', NULL, 'Square Mile Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 94, 81, 93, 2979, 'Medium-Dark', 'Natural', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 87, 59, 79, 2815, 'Espresso', 'Suchá metoda', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Brazílie, Nicaragua', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL28', 83, 51, 53, 2139, 'Light', 'Honey', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 80, 89, 70, 1668, 'Dark', 'Promytá metoda', 'Nicaragua', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie, Peru', 'Lorem Ipsum', NULL, 'Rebelbean', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 82, 51, 96, 2257, 'Medium', 'Washed', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 96, 83, 91, 2750, 'Espresso', 'Anaerobní fermentace', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 91, 86, 88, 2650, 'Dark', 'Suchá metoda', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 82, 50, 95, 1729, 'Light', 'Natural', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 84, 61, 78, 2520, 'Omni', 'Natural', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Etiopie, Rwanda, Kolumbie', 'Lorem Ipsum', NULL, 'Naughty Dog', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 91, 58, 89, 1920, 'Espresso', 'Washed', 'Etiopie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 81, 80, 84, 1380, 'Espresso', 'Washed', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 88, 95, 55, 2616, 'Light', 'Honey', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Etiopie, Rwanda', 'Lorem Ipsum', NULL, 'The Space', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 98, 91, 52, 1455, 'Light', 'Suchá metoda', 'Etiopie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 86, 79, 54, 1710, 'Espresso', 'Natural', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Etiopie', 'Lorem Ipsum', NULL, 'Square Mile Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 87, 52, 64, 2652, 'Dark', 'Promytá metoda', 'Etiopie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Peru, Brazílie', 'Lorem Ipsum', NULL, 'Doubleshot', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 84, 65, 66, 1022, 'Filter', 'Suchá metoda', 'Peru', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 88, 93, 80, 2034, 'Omni', 'Natural', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Rwanda', 'Lorem Ipsum', NULL, 'Rebelbean', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 94, 97, 77, 2151, 'Medium', 'Honey', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie, Peru', 'Lorem Ipsum', NULL, 'Rebelbean', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 92, 66, 86, 1116, 'Medium-Dark', 'Honey', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 80, 98, 96, 1125, 'Medium', 'Honey', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Heirloom', 88, 93, 70, 2317, 'Omni', 'Suchá metoda', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Brazílie', 'Lorem Ipsum', NULL, 'Dabov', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 85, 83, 51, 2718, 'Dark', 'Promytá metoda', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie, Peru', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 97, 79, 55, 2576, 'Medium-Dark', 'Suchá metoda', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 91, 82, 85, 1184, 'Medium-Dark', 'Anaerobní fermentace', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL28', 86, 90, 68, 1773, 'Medium-Dark', 'Anaerobní fermentace', 'Peru', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Panama', 'Lorem Ipsum', NULL, 'Naughty Dog', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 96, 58, 83, 2260, 'Light', 'Washed', 'Panama', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Brazílie, Nicaragua, Panama', 'Lorem Ipsum', NULL, 'Rebelbean', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 91, 95, 78, 1434, 'Filter', 'Washed', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 97, 59, 61, 1282, 'Medium', 'Anaerobní fermentace', 'Nicaragua', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 93, 54, 53, 1710, 'Medium', 'Anaerobní fermentace', 'Panama', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Nicaragua', 'Lorem Ipsum', NULL, 'Rebelbean', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 86, 97, 79, 2874, 'Medium', 'Natural', 'Nicaragua', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Kolumbie, Peru, Brazílie', 'Lorem Ipsum', NULL, 'Doubleshot', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 97, 67, 59, 1845, 'Espresso', 'Natural', 'Kolumbie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 89, 82, 97, 2355, 'Light', 'Honey', 'Peru', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 95, 77, 97, 1391, 'Filter', 'Washed', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Brazílie, Nicaragua, Panama', 'Lorem Ipsum', NULL, 'Doubleshot', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 90, 96, 96, 2374, 'Dark', 'Washed', 'Brazílie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 85, 58, 63, 2450, 'Medium', 'Washed', 'Nicaragua', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 89, 90, 81, 1179, 'Espresso', 'Natural', 'Panama', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Keňa, Etiopie, Rwanda', 'Lorem Ipsum', NULL, 'QB Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 97, 69, 88, 2059, 'Omni', 'Anaerobní fermentace', 'Keňa', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 96, 84, 67, 2459, 'Medium', 'Washed', 'Etiopie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 85, 59, 95, 1524, 'Dark', 'Suchá metoda', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Etiopie, Rwanda', 'Lorem Ipsum', NULL, 'Dabov', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Yellow Bourbon', 83, 88, 73, 2505, 'Espresso', 'Honey', 'Etiopie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL28', 88, 84, 76, 1609, 'Filter', 'Washed', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Etiopie, Rwanda', 'Lorem Ipsum', NULL, 'The Space', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 93, 85, 81, 2667, 'Filter', 'Washed', 'Etiopie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Bourbon', 98, 94, 93, 1998, 'Omni', 'Washed', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Kolumbie', 'Lorem Ipsum', NULL, 'Naughty Dog', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Caturra', 90, 51, 86, 2939, 'Light', 'Washed', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Brazílie', 'Lorem Ipsum', NULL, 'Square Mile Coffee Roasters', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Heirloom', 95, 56, 63, 2928, 'Light', 'Natural', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie', 'Lorem Ipsum', NULL, 'QB Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 86, 52, 75, 2620, 'Espresso', 'Suchá metoda', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 83, 64, 55, 1070, 'Light', 'Promytá metoda', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Rwanda, Kolumbie', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 95, 54, 87, 2979, 'Filter', 'Suchá metoda', 'Rwanda', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 84, 84, 79, 2141, 'Filter', 'Promytá metoda', 'Kolumbie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Rwanda', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('SL34', 89, 63, 95, 1115, 'Espresso', 'Washed', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Keňa', 'Lorem Ipsum', NULL, 'La Boheme Cafe', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Geisha', 92, 87, 97, 2955, 'Medium-Dark', 'Natural', 'Keňa', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Keňa, Etiopie, Rwanda', 'Lorem Ipsum', NULL, 'QB Coffee Roasters', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 82, 83, 68, 1558, 'Omni', 'Honey', 'Keňa', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 99, 87, 78, 2278, 'Omni', 'Promytá metoda', 'Etiopie', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 100, 84, 59, 2870, 'Filter', 'Washed', 'Rwanda', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Blend – Peru, Brazílie', 'Lorem Ipsum', NULL, 'Dabov', 0,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Typica', 93, 93, 94, 2006, 'Medium-Dark', 'Natural', 'Peru', out_i);
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 87, 55, 58, 1485, 'Omni', 'Natural', 'Brazílie', out_i);

    INSERT INTO CoffeeBlend (name, description, short_description, roasters, available, cafe_id)
    VALUES ('Brazílie', 'Lorem Ipsum', NULL, 'QB Coffee Roasters', 1,
            (SELECT id FROM (SELECT id FROM Cafe ORDER BY DBMS_RANDOM.RANDOM) WHERE ROWNUM = 1))
    RETURNING id INTO out_i;
    INSERT INTO CoffeeBean (variety, score, acidity, body, altitude, roast_type, processing_type, area_of_origin,
                            blend_id)
    VALUES ('Catuai', 89, 98, 53, 2471, 'Omni', 'Honey', 'Brazílie', out_i);
end;

-- Roles
INSERT INTO UserCafeRole (user_id, cafe_id, is_owner, position_name)
VALUES (1, 1, 1, 'Majitel');
INSERT INTO UserCafeRole (user_id, cafe_id, is_owner, position_name)
VALUES (2, 1, 0, 'Barista');
INSERT INTO UserCafeRole (user_id, cafe_id, is_owner, position_name)
VALUES (3, 3, 1, 'Majitel');
INSERT INTO UserCafeRole (user_id, cafe_id, is_owner, position_name)
VALUES (4, 5, 1, 'Číšník');

-- Drink offers
INSERT INTO DrinkOffer (name, description, price, cafe_id)
VALUES ('Prvni nabidka', 'Naše skvělá nabídka kávy', 200, 1);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (1, 1);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (3, 1);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (5, 1);

INSERT INTO DrinkOffer (name, description, price, cafe_id)
VALUES ('Druhá nabidka', 'Naše ještě lepší nabídka kávy', 300, 1);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (2, 2);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (4, 2);


INSERT INTO DrinkOffer (name, description, price, cafe_id)
VALUES ('Třetí nabidka', 'Naše nabídka kávy', 300, 3);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (2, 3);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (4, 3);


INSERT INTO DrinkOffer (name, description, price, cafe_id)
VALUES ('Třetí nabidka', 'Naše nabídka kávy', 300, 6);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (7, 4);
INSERT INTO CoffeeBlendsInDrinkOffers (coffee_blend_id, drink_offer_id)
VALUES (9, 4);

-- Ratings and reviews
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (1, 5, 5);
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (1, 6, 2);
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (1, 8, 5);
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (1, 9, 3);
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (2, 5, 4);
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (2, 3, 1);
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (2, 2, 4);
INSERT INTO CafeRating (user_id, cafe_id, rating)
VALUES (4, 10, 5);

INSERT INTO CafeReview (content, date_added, date_of_visit, made_by_user_id, reacts_to_id, reviews_cafe_id)
VALUES ('Skvělá kavárna, příjemná obsluha', TO_DATE('2021-03-20', 'YYYY-MM-DD'), TO_DATE('2021-03-20', 'YYYY-MM-DD'), 1,
        NULL, 1);

INSERT INTO CafeReview (content, date_added, date_of_visit, made_by_user_id, reacts_to_id, reviews_cafe_id)
VALUES ('Naprosto souhlasím!', TO_DATE('2021-03-24', 'YYYY-MM-DD'), TO_DATE('2021-03-20', 'YYYY-MM-DD'), 2, 1, 1);

INSERT INTO CafeReview (content, date_added, date_of_visit, made_by_user_id, reacts_to_id, reviews_cafe_id)
VALUES ('Máte naprosto pravdu', TO_DATE('2021-03-27', 'YYYY-MM-DD'), TO_DATE('2021-03-25', 'YYYY-MM-DD'), 3, 2, 1);

INSERT INTO CafeReview (content, date_added, date_of_visit, made_by_user_id, reacts_to_id, reviews_cafe_id)
VALUES ('Žádný zázrak', TO_DATE('2021-03-27', 'YYYY-MM-DD'), TO_DATE('2021-03-25', 'YYYY-MM-DD'), 4, NULL, 1);

INSERT INTO CafeReview (content, date_added, date_of_visit, made_by_user_id, reacts_to_id, reviews_cafe_id)
VALUES ('Mohlo by to být i lepší', TO_DATE('2021-03-28', 'YYYY-MM-DD'), TO_DATE('2021-03-25', 'YYYY-MM-DD'), 4, NULL,
        5);

INSERT INTO CafeReview (content, date_added, date_of_visit, made_by_user_id, reacts_to_id, reviews_cafe_id)
VALUES ('Příjemné místo', TO_DATE('2021-03-20', 'YYYY-MM-DD'), TO_DATE('2021-03-20', 'YYYY-MM-DD'), 1,
        NULL, 8);

INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (2, 1, 1);
INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (3, 1, 1);
INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (3, 2, 1);
INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (4, 1, 0);
INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (4, 2, 0);
INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (5, 5, 1);
INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (4, 5, 1);
INSERT INTO CafeReviewRating (user_id, review_id, is_positive)
VALUES (3, 5, 1);

-- Cuppings
INSERT INTO CuppingEvent (name, description, date_added, event_date, total_seats, price, cafe_id, made_by_user_id)
VALUES ('Naše první ochutnávka káv', 'Přijďte ochutnat naši nejnovější nabídku káv',
        TO_DATE('2021-03-20', 'YYYY-MM-DD'), TO_DATE('2021-03-27', 'YYYY-MM-DD'), 20,
        300, 10, 1);

INSERT INTO CuppingEvent (name, description, date_added, event_date, total_seats, price, cafe_id, made_by_user_id)
VALUES ('Naše druhá ochutnávka káv', 'Přijďte ochutnat naši nejnovější nabídku káv',
        TO_DATE('2021-05-20', 'YYYY-MM-DD'), TO_DATE('2021-05-27', 'YYYY-MM-DD'), 20,
        400, 10, 1);

INSERT INTO CuppingEvent (name, description, date_added, event_date, total_seats, price, cafe_id, made_by_user_id)
VALUES ('Naše první ochutnávka káv', 'Přijďte ochutnat naši nejnovější nabídku káv',
        TO_DATE('2021-03-27', 'YYYY-MM-DD'), TO_DATE('2021-04-05', 'YYYY-MM-DD'), 40,
        300, 8, 2);

INSERT INTO CuppingEvent (name, description, date_added, event_date, total_seats, price, cafe_id, made_by_user_id)
VALUES ('Naše první ochutnávka káv', 'Přijďte ochutnat naši nejnovější nabídku káv',
        TO_DATE('2021-06-27', 'YYYY-MM-DD'), TO_DATE('2021-07-05', 'YYYY-MM-DD'), 50,
        500, 5, 4);

INSERT INTO CuppingEvent (name, description, date_added, event_date, total_seats, price, cafe_id, made_by_user_id)
VALUES ('Naše první ochutnávka káv', 'Přijďte ochutnat naši nejnovější nabídku káv',
        TO_DATE('2021-07-27', 'YYYY-MM-DD'), TO_DATE('2021-08-05', 'YYYY-MM-DD'), 30,
        400, 3, 3);

INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (1, 1);
INSERT INTO CuppingEventOffer (event_id, blend_id, additional_info)
VALUES (1, 3, 'Informace ke kávě');
INSERT INTO CuppingEventOffer (event_id, blend_id, additional_info)
VALUES (1, 10, 'Informace ke kávě 2');

INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (2, 5);
INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (2, 6);
INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (2, 7);
INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (2, 8);

INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (3, 2);

INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (4, 1);
INSERT INTO CuppingEventOffer (event_id, blend_id)
VALUES (4, 5);

INSERT INTO EventReservation (email, event_id, phone, date_confirmed, made_by_user_id, confirmed_by_user_id)
VALUES ('test@test.com', 1, 608345987, TO_DATE('2021-03-21', 'YYYY-MM-DD'), 2, 1);

INSERT INTO EventReservation (email, event_id, phone, date_confirmed, made_by_user_id, confirmed_by_user_id)
VALUES ('test2@test.com', 1, 603345987, TO_DATE('2021-03-22', 'YYYY-MM-DD'), 3, 1);

INSERT INTO EventReservation (email, event_id, phone, date_confirmed, made_by_user_id, confirmed_by_user_id)
VALUES ('test3@test.com', 1, 603345988, TO_DATE('2021-03-23', 'YYYY-MM-DD'), 5, 1);

INSERT INTO EventReservation (email, event_id, phone, date_confirmed, made_by_user_id, confirmed_by_user_id)
VALUES ('test4@test.com', 3, 348256921, TO_DATE('2021-03-28', 'YYYY-MM-DD'), 1, 2);

-- Cupping reviews
INSERT INTO CuppingEventReview (content, date_added, made_by_user_id, reacts_to_id, reviews_event_id)
VALUES ('Skvělá akce!', TO_DATE('2021-04-01', 'YYYY-MM-DD'), 2, NULL, 1);

INSERT INTO CuppingEventReview (content, date_added, made_by_user_id, reacts_to_id, reviews_event_id)
VALUES ('Jen mohl být trochu větší klid', TO_DATE('2021-04-02', 'YYYY-MM-DD'), 3, 1, 1);

INSERT INTO CuppingEventReview (content, date_added, made_by_user_id, reacts_to_id, reviews_event_id)
VALUES ('Skvělá akce!', TO_DATE('2021-04-01', 'YYYY-MM-DD'), 1, NULL, 3);

INSERT INTO CuppingEventReviewRating (user_id, review_id, is_positive)
VALUES (3, 1, 1);
INSERT INTO CuppingEventReviewRating (user_id, review_id, is_positive)
VALUES (4, 1, 1);
INSERT INTO CuppingEventReviewRating (user_id, review_id, is_positive)
VALUES (5, 2, 1);
INSERT INTO CuppingEventReviewRating (user_id, review_id, is_positive)
VALUES (5, 3, 0);
INSERT INTO CuppingEventReviewRating (user_id, review_id, is_positive)
VALUES (3, 3, 0);
INSERT INTO CuppingEventReviewRating (user_id, review_id, is_positive)
VALUES (4, 3, 0);

COMMIT;

------ DOTAZY ------
/*
-- Jaké cuppingové akce se konaly v kavárně 'Zemanova kavárna a cukrárna'?
SELECT E.name, E.description, E.event_date
FROM Cafe C
         JOIN CuppingEvent E ON C.id = E.cafe_id
WHERE C.name = 'Zemanova kavárna a cukrárna';

-- Jaké recenze má kavárna 'Café Flexaret'?
SELECT R.content, R.date_added
FROM CafeReview R
         JOIN Cafe C ON R.reviews_cafe_id = C.id
WHERE C.name = 'Café Flexaret';

-- Kteří uživatelé (emaily) měli rezervaci na cuppingovou akci 'Naše první ochutnávka káv'
-- v kavárně 'Zemanova kavárna a cukrárna'?
SELECT email
FROM EventReservation R
         JOIN CuppingEvent E on R.event_id = E.id
         JOIN Cafe C on E.cafe_id = C.id
WHERE C.name = 'Zemanova kavárna a cukrárna'
  AND E.name = 'Naše první ochutnávka káv';

-- Kteří uživatelé vlastní kavárnu (e-mail uživatele a název kavárny)?
SELECT U.email, C.name
FROM SUser U
         JOIN UserCafeRole R on U.id = R.user_id
         JOIN Cafe C on R.cafe_id = C.id
WHERE R.is_owner = 1;

-- Kolik majitelů mají kavárny?
SELECT C.name, COUNT(U.ID) owner_count
FROM Cafe C
         LEFT JOIN UserCafeRole R on C.id = R.cafe_id
         LEFT JOIN SUser U on U.id = R.user_id
WHERE R.is_owner = 1
   OR r.is_owner IS NULL
GROUP BY C.name
ORDER BY owner_count DESC;

-- Jakou průměrnou cenu má nabídka nápojů jednotlivých kaváren?
SELECT C.name, AVG(D.price)
FROM Cafe C
         JOIN DrinkOffer D ON C.id = D.cafe_id
GROUP BY C.id, C.name;

-- Kolik uživatelů si oblíbilo jednotlivé typy nápojů? (Ignoruje rozdílné velikosti písmen.)
SELECT LOWER(U.favourite_drink), COUNT(U.id)
FROM SUser U
WHERE U.favourite_drink IS NOT NULL
GROUP BY LOWER(U.favourite_drink);

-- Jaká je průměrná cena a skóre zrn na všech cuppingových akcích navštívených jednotlivými uživateli?
SELECT U.id, U.email, ROUND(AVG(bean.score), 1) average_bean_score, AVG(event.PRICE) average_event_price
FROM SUser U
         JOIN EventReservation e on U.id = e.made_by_user_id
         JOIN CuppingEvent event ON e.event_id = event.id
         JOIN CuppingEventOffer offer ON offer.event_id = e.event_id
         JOIN CoffeeBlend blend ON blend.id = offer.blend_id
         JOIN CoffeeBean bean ON blend.id = bean.blend_id
GROUP BY U.id, U.email;

-- Kteří uživatelé vytvořili recenze u alespoň 2 různých kaváren a kolik recenzí celkem vytvořili?
SELECT U.email, COUNT(R.id) reviews_count
FROM SUser U
         JOIN CafeReview R on U.id = R.made_by_user_id
WHERE EXISTS(SELECT U2.id
             FROM SUser U2
                      JOIN CafeReview R2 ON U2.id = R2.made_by_user_id
             WHERE U.id = U2.id
               AND R2.reviews_cafe_id <> R.reviews_cafe_id)
GROUP BY U.email;

-- Kteří uživatelé ohodnotili cuppingovou akci, ale neohodnotili příslušnou kavárnu (přímo, top-level recenzí)?
SELECT U.id, U.email
FROM SUser U
         JOIN CuppingEventReview CER on U.id = CER.made_by_user_id
         JOIN CuppingEvent CE ON CER.reviews_event_id = CE.id
WHERE U.id NOT IN (SELECT U2.id
                   FROM SUser U2
                            JOIN CafeReview CR ON U2.id = CR.made_by_user_id
                   WHERE U.id = U2.id
                     AND CR.reviews_cafe_id = CE.cafe_id
                     AND CR.reacts_to_id IS NULL);
*/

------ DEMONSTRACE TRIGGERŮ ------
-- 1) Trigger pro generování PK u Cafe
--    Tento trigger má zajišťovat, že ID přidaných kaváren budou vždy sekvenční
DECLARE
    v_id_1 NUMBER;
    v_id_2 NUMBER;
    v_id_3 NUMBER;
BEGIN
    -- Zjistíme aktuální nejvyšší ID:
    SELECT MAX(ID) INTO v_id_1 FROM Cafe;
    DBMS_OUTPUT.PUT_LINE('Aktuální největší ID v Cafe: ' || v_id_1);
    -- Přidáme záznam bez určení ID:
    INSERT INTO Cafe (name, address_line, city) VALUES ('Test 1', 'Test 1', 'Test');
    -- Zjistíme aktuální nejvyšší ID:
    SELECT MAX(ID) INTO v_id_2 FROM Cafe;
    DBMS_OUTPUT.PUT_LINE('Největší ID v Cafe po vložení: ' || v_id_2);
    -- Přidáme záznam s explicitně určeným ID:
    INSERT INTO Cafe (id, name, address_line, city) VALUES (12345, 'Test 2', 'Test 2', 'Test');
    -- Zjistíme aktuální nejvyšší ID:
    SELECT MAX(ID) INTO v_id_3 FROM Cafe;
    DBMS_OUTPUT.PUT_LINE('Největší ID v Cafe po vložení: ' || v_id_3);

    IF v_id_2 = v_id_1 + 1 AND v_id_3 = v_id_2 + 1 THEN
        DBMS_OUTPUT.PUT_LINE('Vložené záznamy mají sekvenční ID, trigger funguje správně.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Vložené záznamy NEmají sekvenční ID, trigger nefunguje správně!');
    end if;
END;

-- 2) Trigger pro generování dne v týdnu pro OpeningHours.
--    Tento trigger zajišťuje, že pokud je do OpeningHours vkládán záznam s day_of_week = NULL,
--    v tabulce se najde poslední záznam pro poslední vyplněný den a day_of_week vkládaného záznamu se nastaví na
--    hodnotu představující další den. (Tedy už v systému mám otevírací hodiny pro čtvrtek, další vložené se automaticky
--    nastaví na pátek.)
DECLARE
    a NUMBER;
    b NUMBER;
    c NUMBER;
    d NUMBER;
BEGIN
    DELETE
    FROM OpeningHours
    WHERE cafe_id = 1
       OR cafe_id = 2;

    -- První vložený den je 5 (sobota)
    INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
    VALUES (1, 5, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI'));

    -- Vložený záznam by měl mít den = 6 (neděle)
    INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
    VALUES (1, NULL, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI'))
    RETURNING day_of_week INTO a;

    -- Vložený záznam by měl mít den = 0 (pondělí)
    INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
    VALUES (1, NULL, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI'))
    RETURNING day_of_week INTO b;

    -- Pro jinou kavárnu začneme od dne 1 (úterý)
    INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
    VALUES (2, 1, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI'));

    -- Vložený záznam by měl mít den = 2 (středa)
    INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
    VALUES (2, NULL, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI'))
    RETURNING day_of_week INTO c;

    -- Nyní vložíme záznam se specifikovaným dnem 5 (sobota)
    INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
    VALUES (2, 5, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI'));

    -- Další záznam bude pro den 6 (neděli)
    INSERT INTO OpeningHours (cafe_id, day_of_week, time_from, time_to)
    VALUES (2, NULL, TO_DATE('8:00', 'HH24:MI'), TO_DATE('17:00', 'HH24:MI'))
    RETURNING day_of_week INTO d;

    IF a = 6 AND b = 0 AND c = 2 AND d = 6 THEN
        DBMS_OUTPUT.PUT_LINE('Vygenerované hodnoty dne v týdnu jsou správné.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Vygenerované hodnoty dne v týdnu NEjsou správné!');
    END IF;

    -- Pro demonstraci, že se transakce zvládne provést i při odstraňování nějaké z kaváren.
    DELETE
    FROM Cafe
    WHERE id = 1;

    COMMIT;
END;

------ DEMONSTRACE PROCEDUR ------
-- 1) set_fav_drink
DECLARE
    a NUMBER;
    b NUMBER;
    c SUser.favourite_drink%TYPE;
    d SUser.favourite_drink%TYPE := 'Nový název nejlepšího nápoje';
BEGIN
    SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;

    INSERT INTO DrinkOffer (name, description, price, cafe_id)
    VALUES ('Nejlepší nápoj', NULL, 100, 2)
    RETURNING id INTO a;

    INSERT INTO DrinkOffer (name, description, price, cafe_id)
    VALUES ('Ještě lepší nápoj', NULL, 100, 2)
    RETURNING id INTO b;

    CafeUtils.SET_FAV_DRINK(1, a);
    CafeUtils.SET_FAV_DRINK(1, b);
    UPDATE DrinkOffer SET name = d WHERE id = a;
    CafeUtils.SET_FAV_DRINK(1, a);
    COMMIT;

    SELECT favourite_drink INTO c FROM SUser WHERE id = 1;
    IF c = d THEN
        DBMS_OUTPUT.PUT_LINE('Vložená hodnota oblíbeného nápoje je správná: ' || c || '.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Vložená hodnota oblíbeného nápoje NENÍ správná: ' || c || '!');
    END IF;
END;

-- 2) set_cafe_review_rating
DECLARE
    a INTEGER;
    e_deadlock EXCEPTION;
    PRAGMA EXCEPTION_INIT (e_deadlock, -00060);
BEGIN
    -- 1. Neexistující uživatel
    DBMS_OUTPUT.PUT_LINE('Následuje varování:');
    CafeUtils.SET_CAFE_REVIEW_RATING(99999, 1, 1);
    -- 2. Neexistující rating
    DBMS_OUTPUT.PUT_LINE('Následuje varování:');
    CafeUtils.SET_CAFE_REVIEW_RATING(2, 99999, 1);
    -- 3. Běžné provedení, kombinace se SELECTem
    DBMS_OUTPUT.PUT_LINE('Následuje nastavení hodnoty na 0, výpis hodnoty, změna na 1 a výpis nové:');
    CafeUtils.SET_CAFE_REVIEW_RATING(2, 5, 0);
    SELECT is_positive INTO a FROM CafeReviewRating WHERE user_id = 2 AND review_id = 5;
    DBMS_OUTPUT.PUT_LINE('Předchozí rating: ' || a);
    CafeUtils.SET_CAFE_REVIEW_RATING(2, 5, 1);
    SELECT is_positive INTO a FROM CafeReviewRating WHERE user_id = 2 AND review_id = 5;
    DBMS_OUTPUT.PUT_LINE('Aktuální rating: ' || a);
    -- 4. Ukázka deadlocku: pokud se pokusíme v rámci jedné transakce přistupovat do tabulky
    --    a potom zavoláme SET_CAFE_REVIEW_RATING, autonomní transakci uvnitř procedury se nepodaří
    --    získat zámek nad tabulkou CafeReviewRating
    DBMS_OUTPUT.PUT_LINE('Následuje deadlock:');
    BEGIN
        UPDATE CafeReviewRating SET is_positive = 0 WHERE user_id = 2 AND review_id = 5;
        CafeUtils.SET_CAFE_REVIEW_RATING(2, 5, 0);
    EXCEPTION
        WHEN e_deadlock THEN
            DBMS_OUTPUT.PUT_LINE('Nastal očekávaný deadlock!');
            ROLLBACK;
    END;
    SELECT is_positive INTO a FROM CafeReviewRating WHERE user_id = 2 AND review_id = 5;
    DBMS_OUTPUT.PUT_LINE('Aktuální rating (očekávaná hodnota je 1): ' || a);

    -- 5. Příklad výše bez deadlocku – po explicitní práci s tabulkou ukončíme transakci.
    DBMS_OUTPUT.PUT_LINE('Následuje ruční změna na 0, commit a potom změna procedurou na 1');
    UPDATE CafeReviewRating SET is_positive = 0 WHERE user_id = 2 AND review_id = 5;
    COMMIT;
    SELECT is_positive INTO a FROM CafeReviewRating WHERE user_id = 2 AND review_id = 5;
    DBMS_OUTPUT.PUT_LINE('Aktuální rating (očekávaná hodnota je 0): ' || a);
    CafeUtils.SET_CAFE_REVIEW_RATING(2, 5, 1);
    SELECT is_positive INTO a FROM CafeReviewRating WHERE user_id = 2 AND review_id = 5;
    DBMS_OUTPUT.PUT_LINE('Aktuální rating (očekávaná hodnota je 1): ' || a);
END;

-- 3) make_reservation
DECLARE
    v_result BOOLEAN;
    v_mail   SUser.email%TYPE;
    a        INTEGER;
    b        DATE;
    c        EventReservation.phone%TYPE;
BEGIN
    DBMS_OUTPUT.PUT_LINE('Vytvoření rezervace na neexistující akci:');
    v_result := CafeUtils.make_reservation(999999, 'neexistujici@uzivatel.cz', 123456789, 10);
    DBMS_OUTPUT.PUT_LINE('Rezervace vytvořena: ' || (CASE v_result WHEN TRUE THEN 'ano' WHEN FALSE THEN 'ne' END));

    DBMS_OUTPUT.PUT_LINE('Vytvoření rezervace na existující akci:');
    v_result := CafeUtils.make_reservation(1, 'neexistujici@uzivatel.cz', 123456789, 10);
    DBMS_OUTPUT.PUT_LINE('Rezervace vytvořena: ' || (CASE v_result WHEN TRUE THEN 'ano' WHEN FALSE THEN 'ne' END));


    DBMS_OUTPUT.PUT_LINE('Vytvoření rezervace na tutéž akci s e-mailem, který přísluší uživateli:');
    SELECT email INTO v_mail FROM SUser WHERE ID = 3;
    v_result := CafeUtils.make_reservation(1, v_mail, 123456789, 5);
    DBMS_OUTPUT.PUT_LINE('Rezervace vytvořena: ' || (CASE v_result WHEN TRUE THEN 'ano' WHEN FALSE THEN 'ne' END));
    SELECT made_by_user_id INTO a FROM EventReservation WHERE email = v_mail AND event_id = 1;
    DBMS_OUTPUT.PUT_LINE('made_by_user_id (očekávaná hodnota 3): ' || a);

    DBMS_OUTPUT.PUT_LINE('Vytvoření rezervace na tutéž akci s příliš velkým požadavkem na místa:');
    v_result := CafeUtils.make_reservation(1, 'dalsi@uzivatel.cz', 123456789, 100);
    DBMS_OUTPUT.PUT_LINE('Rezervace vytvořena: ' || (CASE v_result WHEN TRUE THEN 'ano' WHEN FALSE THEN 'ne' END));

    DBMS_OUTPUT.PUT_LINE(
            'Aktualizace rezervace se stejným počtem míst – nezmění se datum vytvoření, ale změní se telefon:');
    COMMIT;
    DELETE FROM EventReservation WHERE email = 'jiny@uzivatel.com' AND event_id = 1;
    INSERT INTO EventReservation(email, event_id, phone, date_created, number_of_seats, date_confirmed, made_by_user_id,
                                 confirmed_by_user_id)
    VALUES ('jiny@uzivatel.com', 1, 100100100, TO_DATE('2021-03-27', 'YYYY-MM-DD'), 2, CURRENT_DATE, NULL, NULL);
    COMMIT;

    v_result := CafeUtils.make_reservation(1, 'jiny@uzivatel.com', 123456789, 2);
    DBMS_OUTPUT.PUT_LINE('Rezervace vytvořena: ' || (CASE v_result WHEN TRUE THEN 'ano' WHEN FALSE THEN 'ne' END));

    SELECT phone INTO c FROM EventReservation WHERE email = 'jiny@uzivatel.com' AND event_id = 1;
    SELECT date_created INTO b FROM EventReservation WHERE email = 'jiny@uzivatel.com' AND event_id = 1;

    DBMS_OUTPUT.PUT_LINE('Očekávaný telefon je 123456789, skutečný je: ' || c);
    DBMS_OUTPUT.PUT_LINE('Očekávané datum vytvoření je 27.03.21, skutečné je: ' || b);

    DBMS_OUTPUT.PUT_LINE('Aktualizace při nově vytvořeném uživateli:');
    DELETE FROM SUser WHERE email = 'neexistujici@uzivatel.cz';
    INSERT INTO SUser (email, password)
    VALUES ('neexistujici@uzivatel.cz', 'sadkasldjkasld')
    RETURNING id INTO a;
    COMMIT;
    DBMS_OUTPUT.PUT_LINE('ID nově vytvořeného uživatele: ' || a);
    v_result := CafeUtils.make_reservation(1, 'neexistujici@uzivatel.cz', 123456789, 10);
    DBMS_OUTPUT.PUT_LINE('Rezervace vytvořena: ' || (CASE v_result WHEN TRUE THEN 'ano' WHEN FALSE THEN 'ne' END));
    SELECT made_by_user_id INTO a FROM EventReservation WHERE email = 'neexistujici@uzivatel.cz' AND event_id = 1;
    DBMS_OUTPUT.PUT_LINE('made_by_user_id: ' || a);
    COMMIT;
END;

------ DEMONSTRACE OPTIMALIZACE ------

EXPLAIN PLAN FOR
SELECT C.name, COUNT(R.user_id) owner_count
FROM Cafe C
         LEFT JOIN UserCafeRole R on C.id = R.cafe_id
WHERE R.is_owner = 1
   OR r.is_owner IS NULL
GROUP BY C.name
ORDER BY owner_count DESC;

SELECT PLAN_TABLE_OUTPUT
FROM TABLE (DBMS_XPLAN.DISPLAY());

-- Po tomto prikazu spustit znovu predchozi 2.
-- Tento dotaz zefektivní vyhledávání majitelů, kterých by v praxi v databázi byla naprostá menšina.
CREATE INDEX is_owner_index ON UserCafeRole (user_id, cafe_id, is_owner);

EXPLAIN PLAN FOR
SELECT C.name, COUNT(R.user_id) owner_count
FROM Cafe C
         LEFT JOIN UserCafeRole R on C.id = R.cafe_id
WHERE R.is_owner = 1
   OR r.is_owner IS NULL
GROUP BY C.name
ORDER BY owner_count DESC;

SELECT PLAN_TABLE_OUTPUT
FROM TABLE (DBMS_XPLAN.DISPLAY());