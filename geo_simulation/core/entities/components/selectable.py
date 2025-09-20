"""Selectable component"""

# Module implementation

class Selectable:
    def __init__(self):
        self.selected = False

    def select(self):
        self.selected = True

    def deselect(self):
        self.selected = False
