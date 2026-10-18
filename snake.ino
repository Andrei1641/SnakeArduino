#include "TFT.h"
#include "SPI.h"

#define SIZE 10
#define cs 10
#define dc 9
#define rst 8

#define leftButton 2
#define rightButton 3

class Point
{
  private:
    int16_t x;
    int16_t y;
  public:
    Point()
    {
      this->x = 0;
      this->y = 0;
    }
    Point(int16_t x, int16_t y)
    {
      this->x = x;
      this->y = y;
    }

    bool hasSamePosition(Point point)
    {
      if(this->x == point.getX() && this->y == point.getY())
      {
        return true;
      }else
      {
        return false;
      }
    }

    void setX(int16_t x)
    {
      this->x = x;
    }

    int16_t getX()
    {
      return this->x;
    }

    void setY(int16_t y)
    {
      this->y = y;
    }

    int16_t getY()
    {
      return this->y;
    }
};

class Node
{
  private:
    Point point;
    Node* nextNode = nullptr;
    Node* previousNode = nullptr;

  public:
    Node(int16_t x, int16_t y)
    {
      this->point = Point(x, y);
    }

    Point getPoint()
    {
      return this->point;
    }

    void setPoint(Point point)
    {
      this->point = point;
    }

    Node* getNextNode()
    {
      return this->nextNode;
    }

    Node* getPreviousNode()
    {
      return this->previousNode;
    }

    void setNextNode(Node* nextNode)
    {
      this->nextNode = nextNode;
    }

    void setPreviousNode(Node* previousNode)
    {
      this->previousNode = previousNode;
    }
};

class LinkedList
{
  private:
    Node* first;
    Node* last;

  public:
    LinkedList(Node& first)
    {
      this->first = &first;
      this->last = &first;
    }

    Node* getFirstNode()
    {
      return this->first;
    }

    Node* getLastNode()
    {
      return this->last;
    }

    void setLastNodeOnFirstPlace()
    {
      Node* lastTmp = this->getLastNode();
      Node* secondLastTmp = lastTmp->getPreviousNode();
      Node* firstTmp = this->getFirstNode();

      lastTmp->setPreviousNode(nullptr);
      lastTmp->setNextNode(firstTmp);
      this->first = lastTmp;

      secondLastTmp->setNextNode(nullptr);
      this->last = secondLastTmp;

      firstTmp->setPreviousNode(lastTmp);
    }

    void setFirstNode(Node* newFirstNode)
    {
      this->first->setPreviousNode(newFirstNode);
      newFirstNode->setNextNode(this->first);
      this->first = newFirstNode;
    }
};

class Snake
{
  private:
    Node firstPart{2 * SIZE, 0};
    Node secondPart{SIZE, 0};
    Node thirdPart{0, 0};
    bool growing = false;

    LinkedList snakeparts = LinkedList(thirdPart);
    
    uint8_t direction = 1;      //1-reight 2-down 3-left 4-up
    
  public:
    Snake()
    {
      this->snakeparts.setFirstNode(&secondPart);
      this->snakeparts.setFirstNode(&firstPart);
    }

    Node* getHead()
    {
      return this->snakeparts.getFirstNode();
    }

    Node* getTail()
    {
      return this->snakeparts.getLastNode();
    }

    void setGrowing(bool growing)
    {
      this->growing = growing;
    }

    bool isGrowing()
    {
      return this->growing;
    }

    void move()
    {
      Node* head = this->getHead();
      Point headPoint = head->getPoint();
      Node* tail = this->getTail();
      Point tmpPoint;

      if(this->direction > 4)
      {
        this->direction = 1;
      }
      if(this->direction < 1)
      {
        this->direction = 4;
      }

      switch(this->direction)
      {
        case 1:
          tmpPoint = Point(headPoint.getX() + SIZE, headPoint.getY());
          break;
        case 2:
          tmpPoint = Point(headPoint.getX(), headPoint.getY() + SIZE);
          break;
        case 3:
          tmpPoint = Point(headPoint.getX() - SIZE, headPoint.getY());       
          break;
        case 4:
          tmpPoint = Point(headPoint.getX(), headPoint.getY() - SIZE);
          break;
      }

      if(!this->growing)
      {
        tail->setPoint(tmpPoint);
        this->snakeparts.setLastNodeOnFirstPlace();
      }else
      {
        Node* newSnakePart = new Node(tmpPoint.getX(), tmpPoint.getY());
        this->snakeparts.setFirstNode(newSnakePart);
        this->growing = false;
      }

    }

    void changeDirection(uint8_t leftRight)   //0 for left, 1 for right
    {
      if(leftRight == 0)
      {
        this->direction--;
      }else if(leftRight == 1)
      {
        this->direction++;
      }
    }
};

class Apple
{
  private:
    Point position;
  public:
    void setRandomPosition(uint16_t frameHigth, uint16_t frameWidth)
    {
      this->position.setX(random(frameWidth / SIZE) * SIZE);
      this->position.setY(random(frameHigth / SIZE) * SIZE);
    }

    Point getPosition()
    {
      return this->position;
    }
};

TFT screen = TFT(cs, dc, rst);
Snake snake;
Apple apple;

bool game = true;

volatile int8_t buttonChangeDirection = 0; //-1 left, 1 right, 0 no change direction
uint32_t lastTimePressed = 0;

void turnLeft()
{
  if(millis() - lastTimePressed > 300)
  {
    buttonChangeDirection = -1;
    lastTimePressed = millis();
  }
}

void turnRight()
{
  if(millis() - lastTimePressed > 300)
  {
    buttonChangeDirection = 1;
    lastTimePressed = millis();
  }
}

void setup() 
{
  Serial.begin(9600);

  pinMode(leftButton, INPUT_PULLUP);
  pinMode(rightButton, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2), turnLeft, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), turnRight, FALLING);

  screen.begin();
  screen.background(0, 0, 0);

  apple.setRandomPosition(screen.height(), screen.width());
}

void loop() 
{
  if(game == true)
  {
//--------change direction of the snake--------
    if(buttonChangeDirection == -1)
    {
      snake.changeDirection(0);
      buttonChangeDirection = 0;
    }
    if(buttonChangeDirection == 1)
    {
      snake.changeDirection(1);
      buttonChangeDirection = 0;
    }
//---------------------------------------------
    snake.move();
//--------go through every snake part----------
    screen.stroke(255, 0, 0);
    Node* headPart = snake.getHead();
    Node* tmpSnakePart = snake.getHead();
    while (tmpSnakePart != nullptr)
    {
      Point tmpPoint = tmpSnakePart->getPoint();
      if(headPart != tmpSnakePart)
      {
        Point headPoint = headPart->getPoint();
        if(headPoint.hasSamePosition(tmpPoint))
        {
          game = false;
        }
      }
      screen.rect(tmpPoint.getX(), tmpPoint.getY(), SIZE, SIZE);
      tmpSnakePart = tmpSnakePart->getNextNode();
    }
//---------------------------------------------
    Point headPoint = snake.getHead()->getPoint();
//--------snake eats an apple------------------
    if(headPoint.hasSamePosition(apple.getPosition()))
    {
      snake.setGrowing(true);
      apple.setRandomPosition(screen.height(), screen.width());
    }
//---------------------------------------------
//--------snake is out the screen--------------
    if(headPoint.getX() > screen.width() || headPoint.getX() < 0 || headPoint.getY() > screen.height() || headPoint.getY() < 0)
    {
      game = false;
    }
//---------------------------------------------
//--------render apple-------------------------
    screen.stroke(0, 0, 255);
    Point applePosition = apple.getPosition();
    screen.rect(applePosition.getX(), applePosition.getY(), SIZE, SIZE);
//---------------------------------------------
    delay(1000);
//--------reload screen by erasing last part of the snake (tail)
    screen.stroke(0, 0, 0);
    Node* tail = snake.getTail();
    Point tailPoint = tail->getPoint();
    screen.rect(tailPoint.getX(), tailPoint.getY(), SIZE, SIZE);
//---------------------------------------------
  } else
  {
//--------game over----------------------------
    screen.stroke(0, 0, 255);
    screen.text("game over", 0, 0);
//---------------------------------------------
  }
}