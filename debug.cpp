#include "lib/game.h"
#include "lib/input.h"
#include <algorithm>
#include <chrono>
#include <string>

static constexpr int FPS{60};
static bool gameover{};
static int score{};

class Bullet
{
public:
    Bullet() 
    {
        alive = false;
    }

    Bullet(int xStart, int yStart, int _xEnd, int _yEnd)
    {
        x = xStart;
        y = yStart;
        xEnd = _xEnd;
        yEnd = _yEnd;
        int xDist = abs(xEnd-x);
        int yDist = abs(yEnd-y);
        int maxDist = xDist > yDist ? xDist : yDist;
        xSpeed = (xEnd-x) / maxDist;
        ySpeed = (yEnd-y) / maxDist;
    }

    void Update()
    {
        if (!alive) return;

        grid.SetTile(x, y);

        float xNew = x+xSpeed;
        float yNew = y+ySpeed;
        if (grid.IsOutOfBounds(xNew, yNew))
        {
            alive = false;
            return;
        }

        x = xNew;
        y = yNew;

        grid.SetTile(x, y, ascii);
    }

    bool IsAlive() const { return alive; }

    static constexpr char ascii = '*';

private:
    float x{};
    float y{};
    int xEnd{};
    int yEnd{};
    float xSpeed{};
    float ySpeed{};
    bool alive{true};
};

class Player
{
public:
    void Update()
    {
        grid.SetTile(x, y);

        if (userInput == UserInput::Mouse1)
            bullets.emplace_back(x, y, mouseInput.x, mouseInput.y);
            
        for (auto& bullet : bullets)
            bullet.Update();

        for (int i = 0; i < bullets.size(); i++)
        {
            if (!bullets[i].IsAlive())
            {
                bullets[i] = bullets.back();
                bullets.pop_back();
            }
        }

        grid.SetTile(x, y, ascii);
    }

    int GetX() const { return x; }
    int GetY() const { return y; }

    static constexpr char ascii = '@';

private:
    int x{grid.GetMidWidth()};
    int y{grid.GetMidHeight()};
    vector<Bullet> bullets{};
};

class Bug
{
public:
    Bug(int _xEnd, int _yEnd)
    {
        int roll = rand() % 4;
        switch(roll)
        {
            case 0:
            {
                x = 0;
                y = rand() % grid.GetHeight();
                break;
            }
            case 1:
            {
                x = rand() % grid.GetWidth();
                y = 0;
                break;
            }
            case 2:
            {
                x = grid.GetWidth()-1;
                y = rand() % grid.GetHeight();
                break;
            }
            case 3:
            {
                x = rand() % grid.GetWidth();
                y = grid.GetHeight()-1;
                break;
            }
        }

        xEnd = _xEnd;
        yEnd = _yEnd;
        int xDist = abs(xEnd-x);
        int yDist= abs(yEnd-y);
        maxDistance = xDist > yDist ? xDist : yDist;
        xSpeed = (xEnd-x) / maxDistance;
        ySpeed = (yEnd-y) / maxDistance;
    }

    void Update()
    {
        if (!alive) return;

        if (grid.IsCollision(x, y, Bullet::ascii))
        {
            alive = false;
            score++;
            return;
        }

        if (frames++ != framesPerMove) return;
        frames = 0;

        grid.SetTile(x, y);

        maxDistance--;
        float xNew = !maxDistance ? xEnd : x+xSpeed;
        float yNew = !maxDistance ? yEnd : y+ySpeed;

        if (grid.IsOutOfBounds(xNew, yNew))
        {
            alive = false;
            return;
        }
        else if (grid.IsCollision(xNew, yNew, Player::ascii))
        {
            alive = false;
            gameover = true;
            return;
        }
        
        x = xNew;
        y = yNew;

        grid.SetTile(x, y, ascii);
    }

    bool IsAlive() const { return alive; }

    static constexpr char ascii = 'b';

private:
    float x{};
    float y{};
    float xSpeed{};
    float ySpeed{};
    int xEnd{};
    int yEnd{};
    int maxDistance{};
    bool alive{true};
    int frames{};
    int framesPerMove{6};
};

class BugSpawner
{
public:
    BugSpawner(int _xEnd, int _yEnd)
    {
        xEnd = _xEnd;
        yEnd = _yEnd;
    }

    void Update()
    {
        for (auto& bug : bugs)
            bug.Update();

        for (int i = 0; i < bugs.size(); i++)
        {
            if (!bugs[i].IsAlive())
            {
                bugs[i] = bugs.back();
                bugs.pop_back();
            }
        }

        if (frames++ != framesPerSpawn) return;
        frames = 0;
        bugs.emplace_back(xEnd, yEnd);
    }

private:
    int xEnd{};
    int yEnd{};
    int frames{};
    int framesPerSpawn{FPS};
    vector<Bug> bugs{};
};

class Debug : public Game
{
public:
    Debug() : Game(FPS) 
    {
        srand(time(NULL));
    }

protected:
    void Update()
    {            
        player.Update();
        bugSpawner.Update();
        if (gameover)
            End("GAME OVER! Score " + to_string(score));
            
    }

    Player player{};
    BugSpawner bugSpawner{player.GetX(), player.GetY()};
};

int main()
{
    Debug debug{};
    debug.Start();
    return 0;
}