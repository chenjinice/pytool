# -*- coding: utf-8 -*-
import math

from web.server      import startServer
from asn.asn         import *
from obu.obu         import *


str = 'https://khms1.google.com/kh/v=908?x={x}&y={y}&z={z}'

x='11'
y='22'
z='33'
url = str.replace('{x}',x).replace('{y}',y).replace('{z}',z)
print(url)


if __name__ == '__main__':

    startServer(obuAll,asnAll)



