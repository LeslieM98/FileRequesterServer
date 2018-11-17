## Client->Server package
1. Die ersten 2 byte be schreiben die größe der angeforderten dateien interpretiert als unsigned 16 bit integer.  
2. Die nachfolgenden 2 byte beschreiben die string länge des pfades der angeforderten datei. z.B. für `tmp.txt` wären es 7.  
3. nachfolgend ist der pfad ohne null terminator (`\n`) am ende.

## Server->Client package
1. Das erste byte ist ein statusbyte das als unsigned 8 bit integer zu interpretieren ist.
Mögliche statusse:
   - 0: Alles genau so wie gefordert. Zurück geschickten file bytes sind die exakte anzahl als angefordert.
   - 1: Es konnten nur 0 bytes gelesen werden. Eventuell falls die datei nicht existiert oder kein zugriff möglich ist. (Daten werden nicht gesendet)
   - 2: Es wurden weniger byte zurückgesendet als angefordert wurden. Bspw gefordert sind 10 bytes aber es kommt nur `1234567` zurück.  
2. Die folgenden 2 bytes beschreiben die größe der zurückgesendeten daten. Interpretiert als unsigned 16 bit integer.  
3. Der dateiinhalt wird in der in `2.` beschriebenen größe angehängt ohne nullterminator. Falls der string null terminiert ist, ist ein `00000000` byte innerhalb der datei gelesen worden und gehört einfach zur datei