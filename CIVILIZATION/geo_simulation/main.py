"""Game entry point"""

from core.governance.government import Government
from core.ai.decision_making import DecisionEngine
from core.subunits.subunit_types.state import State
from core.events.event_bus import EventBus

def run_simulation():
    # Setup
    gov = Government("Republic of Arcadia", "democracy")
    gov.add_subunit(State("Northland", 500_000, 12))
    gov.add_subunit(State("Southport", 300_000, 8))

    decision_engine = DecisionEngine()
    event_bus = EventBus()

    # One simulation tick
    taxes = gov.collect_taxes()
    decision = gov.make_decision(decision_engine)

    print(f"Collected Taxes: {taxes}")
    print(f"Decision Taken: {decision}")

if __name__ == "__main__":
    run_simulation()
