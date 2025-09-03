"""Base entity class"""

# Module implementation

class Entity:
    def __init__(self, name):
        self.name = name
        self.components = {}

    def add_component(self, component_name, component):
        self.components[component_name] = component

    def get_component(self, component_name):
        return self.components.get(component_name, None)
