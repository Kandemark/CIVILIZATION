"""AI decision making"""

# Module implementation

import random

class DecisionEngine:
    def __init__(self, event_bus):
        self.event_bus = event_bus

    def evaluate(self, government, context):
        """
        context = dict containing state of economy, population, unrest, etc.
        """
        choices = []

        # Economy-driven choices
        if context["economy"]["deficit"] > 0:
            choices.append(("raise_taxes", context["economy"]["deficit"] * 0.7))
        if context["economy"]["growth"] < 0:
            choices.append(("stimulus", abs(context["economy"]["growth"]) * 0.5))

        # Population unrest
        if context["population"]["unrest"] > 0.6:
            choices.append(("reforms", context["population"]["unrest"] * 1.0))
        if context["population"]["unrest"] > 0.8 and government.type == "dictatorship":
            choices.append(("repression", context["population"]["unrest"] * 1.2))

        # Military pressure
        if context["military"]["threat"] > 0.5:
            choices.append(("mobilize", context["military"]["threat"]))

        # If no choices, do nothing
        if not choices:
            return None

        # Weighted random choice
        decision = random.choices(
            population=[c[0] for c in choices],
            weights=[c[1] for c in choices],
            k=1
        )[0]

        # Emit event
        self.event_bus.emit({
            "type": "decision",
            "government": government.name,
            "action": decision
        })

        return decision
