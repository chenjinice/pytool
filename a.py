import math
import numpy as np
import matplotlib.pyplot as plt


t = 0.1
x = np.matrix([0,1]).T
a = np.matrix([[1,t],[0,1]])
h = np.matrix([[1,0],[0,1]])
p = np.matrix([[0.11,0],[0,0.12]])
q = np.matrix([[0,0],[0,0]])
r = np.matrix([[0.01,0],[0,0.01]])
i = np.matrix([[1,0],[0,1]])


x_arr = []
tx_arr= []
px_arr= []
pv_arr= []
rx_arr= []

def kalman(z_meas):
#   predict
    global x,p,q,h,r
    x           = a*x
    p           = a*p*a.T + q
    tx_arr.append(x[0,0])
#   update
    tmp1        = h*p*h.T + r
    tmp2        = np.linalg.inv(tmp1)
    k           = p*h.T*tmp2
    z_predict   = h*x
    x           = x + k*(z_meas-z_predict)
    p           = (i-k*h)*p

    px_arr.append(x[0,0])
    pv_arr.append(x[1,0])



#size = 10
#a=np.random.randint(5,10,size=size)
#a_avg=np.mean(a)
#print("a =",a)
#print("avg =",a_avg)
#print("s2 = ",np.var(a))
#print("s =",np.std(a))


#b=np.random.randint(5,10,size=size)
#b_avg=np.mean(b)
#print("b =",b)
#print("avg =",b_avg)
#print("s2 = ",np.var(b))

#sum = 0
#for i in range(size):
#    sum+=(a[i]-a_avg)*(b[i]-b_avg)

#print("sum = ",sum/size)


#mt1 = np.matrix([[1,1],[1,1.1]])
#mt2 = np.matrix([[1,2],[3,4]])
#mt3 = np.linalg.inv(mt1)
#print( mt2*mt3 )
#print( mt2/mt1 )
#print( mt3)



for i in range(16):
    x[1,0] = 1 + np.random.randint(10)
    x_arr.append(i)
    rx_arr.append(i)
    z_meas = np.matrix([[i],[1]])

    kalman(z_meas)


plt.figure(20)
plt.plot(x_arr, tx_arr, color='g',linestyle='-')
plt.plot(x_arr, px_arr, color='r',linestyle='-')
plt.plot(x_arr, pv_arr,  color='b',linestyle='--')
plt.plot(x_arr, rx_arr,  color='y',linestyle='-.')
plt.title('0000')
plt.legend(['y1', 'y2', 'y3','y4'])
plt.xlabel('x')
plt.ylabel('y')
plt.show()  # 显示图片
