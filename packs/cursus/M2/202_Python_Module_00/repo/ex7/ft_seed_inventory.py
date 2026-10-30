def ft_seed_inventory(seed_type: str, quantity: int, unit: str) -> None:
    if unit == "packets":
        unit = "packets available"
        covers = ""
    elif unit == "grams":
        unit = "grams total"
        covers = ""
    elif unit == "area":
        unit = "square meters"
        covers = "covers "
    else:
        print("Unknown unit type")
        return
    print(f"{seed_type.capitalize()} seeds: {covers}{quantity} {unit}")
