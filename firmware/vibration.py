# pip install XInput-Python

import XInput

a,b,c,d = XInput.get_connected()

index = -1
if a:
    index = 0
elif b:
    index = 1
elif c:
    index = 2
elif d:
    index = 3


if index >= 0:
    print("select: {0}".format(index))
    while True:
        strVb = input("vibration: ")
        strVbList = strVb.split(" ")
        if len(strVbList) != 2:
            print("invalid input")
            continue

        try:
            left =  float(strVbList[0])
            right =  float(strVbList[1])
        except:
            pass

        print("left: {0}, right: {1}".format(left, right))
        XInput.set_vibration(index, left, right)


