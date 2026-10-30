class Plant:
    def __init__(self, name: str, height: str, age: str) -> None:
        self.name = name
        self.height = height
        self.age = age

    def show(self) -> None:
        print(f"{self.name}: {self.height}, {self.age}")


if __name__ == "__main__":
    plant1 = Plant("Rose", "25cm", "30 days old")
    plant2 = Plant("Sunflower", "80cm", "45 days old")
    plant3 = Plant("Cactus", "15cm", "120 days old")

    print("=== Garden Plant Registry ===")
    plant1.show()
    plant2.show()
    plant3.show()
