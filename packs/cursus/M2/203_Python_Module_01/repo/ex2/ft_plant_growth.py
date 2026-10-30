class Plant:
    def __init__(self, name, height, age):
        self.name = name
        self.height = height
        self.age = age

    def show(self):
        print(f"{self.name}: {self.height}, {self.age}")

    def grow(self, growth_amount):
        self.height = f"{int(self.height[:-2]) + growth_amount}cm"
        self.age = f"{int(self.age.split()[0]) + 1} days old"

    def age(self, days):
        self.age = f"{int(self.age.split()[0]) + days} days old"


if __name__ == "__main__":
    plant1 = Plant("Rose", "25cm", "30 days old")
    plant2 = Plant("Sunflower", "80cm", "45 days old")
    plant3 = Plant("Cactus", "15cm", "120 days")

    print("=== Garden Plant Registry ===")
    plant1.show()
    plant2.show()
    plant3.show()
