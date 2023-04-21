import cx_Oracle

def user_conn():
    user = "waden"
    pwd = "password"
    host = "192.168.0.5"
    #host = "192.168.0.9"
    port = "1521"
    sid = "orcl"

    connection_string = f"{user}/{pwd}@{host}:{port}/{sid}"
    return connection_string

def check(cursor, connection):
    check_table = '''
    SELECT table_name FROM user_tables WHERE table_name = 'DEVICE' 
    OR table_name = 'ENERGYMONITOR'
    '''
    cursor.execute(check_table)
    result = cursor.fetchone()

    if result is None:
        try:
            create_table1 = '''
            CREATE TABLE device(
            sensor_id NUMBER(10) PRIMARY KEY,
            sensor_name VARCHAR2(20) NOT NULL UNIQUE,
            sensor_type VARCHAR2(20) NOT NULL
            ) '''
            create_table2 = '''
            CREATE TABLE energymonitor(
            sensor_time VARCHAR2(20) NOT NULL,
            sensor_name VARCHAR2(20) NOT NULL,
            sensor_value NUMBER(*,1) NOT NULL,
            CONSTRAINT fk_sname FOREIGN KEY (sensor_name) REFERENCES device(sensor_name)
            )'''

            cursor.execute(create_table1)
            cursor.execute(create_table2)

            connection.commit()

            print("Success Create Table")
        except Exception as e:
            print(f"Failed : {e}")
    else:
        print("There is already a table")
        pass

def create_tables():
    conn = user_conn()
    connection = cx_Oracle.connect(conn)
    cursor = connection.cursor()
    check(cursor, connection)