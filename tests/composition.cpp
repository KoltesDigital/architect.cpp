struct Person
{
    int age;
}

class Team
{
public:
    Person *getPerson(int i);

private:
    Person _persons[3];
}

Team team;

class Company
{
public:
    Team *getTeam()
    {
        return &team;
    }

    static Company *instance;
}

int main()
{
    team.getPerson(0);
    //return Company::instance->getTeam()->getPerson(1)->age;
}
