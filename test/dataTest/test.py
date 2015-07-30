#!/usr/bin/python


class Wdb:
    def __init__(self, **kwargs):
        import pg
        self.database = pg.connect(**kwargs)
        self.database.query("SELECT wci.begin('wdb')")
        
    def __del__(self):
        self.database.close()
        
    def read(self, provider, latitude, longitude):
        query = "SELECT value, ST_AsText(placegeometry) FROM wci.read(ARRAY['%s'],'surround POINT(%f %f)',NULL,NULL,NULL,NULL,NULL,NULL::wci.returnfloat)"
        #print(query % (provider,longitude, latitude) )
        result = self.database.query(query % (provider, longitude, latitude)).dictresult()
        ret = []
        for row in result:
            ret.append(row['value'])
        return ret
        
def format(s, spaces):
    return s + (spaces-len(s))*' '

class ValidatedData:
    def __init__(self, inputLine):
        input = inputLine.split()
        self.name = input[0]
        self.latitude = float(input[1])
        self.longitude = float(input[2])
        self.altitude = int(input[3])
        
    def __str__(self):
        return format(self.name, 18) + format(str(self.altitude), 12)
    
    def validate(self, fetchedData):
        #fetchedData.sort()
        print str(self) + str(fetchedData)

def processFile(fileToProcess, provider, database):
    print 'STED              YR.NO       WDB'
    f = file(fileToProcess)
    for line in f:
        line = line.strip()
        if ( len(line) > 0 and line[0] != '#' ):
            line = line.replace(',', '.')
            valid = ValidatedData(line)
            valid.validate(database.read(provider, valid.latitude, valid.longitude))


if __name__ == '__main__':
    import sys

    db = Wdb(host='10.99.2.117', user='wdb', dbname='wdb', port=5432, passwd=None)

    filesToParse = sys.argv[1:]
    if len(filesToParse) == 0:
        filesToParse = ['samples.txt']

    for f in filesToParse:
        processFile(f, 'gmtedtopo2010', db)