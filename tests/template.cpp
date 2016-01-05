template <typename T>
class allocator
{
};

template <typename Key, typename Value, template<typename T> class Alloc = allocator>
class map
{
public:
	map<Key, Value> &operator=(map<Key, Value> &other);
};

struct Component
{
};

template <typename T>
class ComponentMap : public map<T, Component>
{
};

struct Element
{
    ComponentMap<int> components;
};

template <typename T>
void get()
{
    map<T, Element> elements;
}

void getSpecialized()
{
	get<int>();
}
