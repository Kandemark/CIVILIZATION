"""Population composition"""

# Module implementation

class Demographics:
    def __init__(self, total_population):
        self.total_population = total_population

    def add_population(self, amount):
        self.total_population += amount
