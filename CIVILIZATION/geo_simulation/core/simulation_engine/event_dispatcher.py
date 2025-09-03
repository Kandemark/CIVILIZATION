"""Efficient event handling"""

# Module implementation

class EventDispatcher:
    def __init__(self):
        self.events = []

    def schedule_event(self, year, description):
        self.events.append({"year": year, "description": description})

    def process_events(self, current_year):
        for event in self.events:
            if event["year"] == current_year:
                print(f"[Event] {event['description']}")
        # Remove processed events
        self.events = [e for e in self.events if e["year"] > current_year]
