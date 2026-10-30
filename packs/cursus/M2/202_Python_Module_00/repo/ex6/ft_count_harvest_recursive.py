def ft_count_harvest_recursive(days=None):
    is_first_call = False
    if days is None:
        day = int(input("Days until harvest: "))
        is_first_call = True
    else:
        day = days
    if day > 0:
        ft_count_harvest_recursive(day - 1)
        print(f"Day {day}")
    if is_first_call:
        print("Harvest time!")
