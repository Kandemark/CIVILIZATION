"""Event bus pattern"""

# Module implementation

class EventBus:
    def __init__(self):
        self.listeners = []

    def register(self, listener):
        self.listeners.append(listener)

    def emit(self, event):
        for listener in self.listeners:
            listener.notify(event)
