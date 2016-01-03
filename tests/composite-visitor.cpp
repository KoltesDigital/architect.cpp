class Visitor
{
public:
    void declareLeaf();
};

class Node
{
public:
    virtual void accept(Visitor &visitor) = 0;
};

class Leaf : public Node
{
public:
    virtual void accept(Visitor &visitor) override
    {
        visitor.declareLeaf();
    }
};

class Composite : public Node
{
public:
    virtual void accept(Visitor &visitor) override
    {
        for (auto node : _nodes)
            node->accept(visitor);
    }

private:
    Node* _nodes;
    size_t _nodeCount;
};

void visit(Node *root)
{
    Visitor visitor;
    root->accept(visitor);
}
