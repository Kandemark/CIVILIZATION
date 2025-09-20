"""State subunit implementation"""

# Module implementation

class State:
    def __init__(self, name, population, productivity, tax_rate=0.15):
        self.name = name
        self.population = population
        self.productivity = productivity
        self.tax_rate = tax_rate

    def simulate_economy(self):
        # GDP = productivity × population
        return self.population * self.productivity

    def collect_taxes(self):
        gdp = self.simulate_economy()
        return gdp * self.tax_rate
