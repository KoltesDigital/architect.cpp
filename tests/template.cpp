template <typename Key, typename Value>
class map
{
};

struct Component
{
};

struct Element
{
    map<int, Component> components;
};

void main()
{
    map<int, Element> elements;
}
