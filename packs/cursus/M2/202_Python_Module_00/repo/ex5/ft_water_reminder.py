def ft_water_reminder():
    a = input("Days since last watering: ")
    if int(a) > 2:
        print("Water the plants!")
    else:
        print("Plants are fine")
