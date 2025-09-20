"""Base government class"""

# Module implementation

import json
from pathlib import Path

class Government:
    def __init__(self, name, gov_type):
        self.name = name
        self.gov_type = gov_type
        self.subunits = []
        self.decision_priority = []
        self.load_template()

    def load_template(self):
        path = Path("config/governance_templates") / f"{self.gov_type.lower()}.json"
        with open(path, "r") as f:
            template = json.load(f)
        self.decision_priority = template.get("decision_priority", [])

    def add_subunit(self, subunit):
        self.subunits.append(subunit)

    def collect_taxes(self):
        return sum(s.collect_taxes() for s in self.subunits)

    def make_decision(self, decision_engine):
        return decision_engine.evaluate(self)
