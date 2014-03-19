#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include "collision.h"

using namespace std;
using namespace sf;

constexpr float playerVelocity{10.f};
constexpr float playerRotation{10.f};
constexpr float gravity{2.f};

constexpr float enemyVelocity{10.f};
constexpr float enemyDestroyedVelocity{5.f};

constexpr float bulletVelocity{15.f};

int window_width{1920};
int window_height{1080};

bool gameover = false;

/*----------------------DoubleBackground----------------------*/
struct DoubleBackground
{
	Sprite background_1;
	Sprite background_2;
	float default_move_x = 3.f;
	float move_x = default_move_x;

	DoubleBackground(Texture* background_1_texture, Texture* background_2_texture)
	{
		background_1.setTexture(*background_1_texture);
		background_2.setTexture(*background_2_texture);
		background_2.setPosition(-window_width, 0);
	}

	void update()
	{
		Vector2f back1Position = background_1.getPosition();
		Vector2f back2Position = background_2.getPosition();

		if(back2Position.x >= 0)		
			background_1.setPosition(back2Position.x - window_width, 0);

		if(back1Position.x >= 0)		
			background_2.setPosition(back1Position.x - window_width, 0);	

		background_1.move(move_x, 0);
		background_2.move(move_x, 0);
	}
};
/*----------------------DoubleBackground----------------------*/

/*----------------------Enemy----------------------*/
struct Enemy
{
	Sprite sprite;
	Vector2f velocity;
	Texture* t_alive;
	Texture* t_destroyed;

	Enemy(Texture* t_alive, Texture* t_destroyed, float mX, float mY)
	{
		this->t_alive = t_alive;
		this->t_destroyed = t_destroyed;
		sprite.setTexture(*t_alive);
		sprite.setPosition(mX, mY);
	}

	void update()
	{
		if(sprite.getPosition().x > window_width)
		{
			float posX = 0;
			float posY = (float)(rand()%window_height); 
			sprite.setPosition(posX, posY);
		}

		velocity.x = enemyVelocity;
		sprite.move(velocity);
	}
};
/*----------------------Enemy----------------------*/

/*----------------------EnemyDestroyed----------------------*/
struct EnemyDestroyed
{
	Sprite sprite;
	Vector2f velocity;
	Clock clock_disappear; //il clock parte non appena viene costruito l'oggetto
	float disappear_after = 0.5f;
	Clock clock_scale;
	float scale_after = disappear_after/10.f;

	EnemyDestroyed(Sprite sprite)
	{
		this->sprite = sprite;
	}

	bool mustDisappear()
	{
		Time elapsed = clock_disappear.getElapsedTime();

		return elapsed.asSeconds() >= disappear_after;
	}

	void update()
	{
		velocity.x = enemyDestroyedVelocity;
		sprite.move(velocity);

		if(clock_scale.getElapsedTime().asSeconds() >= scale_after)
		{
			Vector2f scalingFactor(1.1f, 1.1f); //l'esplosione diventa gradualmente più grande
			sprite.scale(scalingFactor);
			clock_scale.restart();
		}
	}
};
/*----------------------EnemyDestroyed----------------------*/

/*----------------------EnemyManager----------------------*/
struct EnemyManager
{
	vector<Enemy> enemies;
	vector<EnemyDestroyed> enemies_destroyed;

	EnemyManager()
	{

	}

	void addEnemy(Texture* t_alive, Texture* t_destroyed, float mX, float mY)
	{
		enemies.emplace_back(t_alive, t_destroyed, mX, mY);
	}

	void createEnemyRandomPosition(Texture* t_alive, Texture* t_destroyed)
	{
	    Vector2u textureSize = t_alive->getSize();        
	    int enemyHeight = (int)(rand()%(window_height - textureSize.y*2) + textureSize.y*2);
	    //Enemy enemy1(&enemy_texture, enemyTextureSize.x, window_height/2 - enemyTextureSize.y);
		enemies.emplace_back(t_alive, t_destroyed, textureSize.x, enemyHeight);
	}

	void collision(int indexToRemove)
	{
		Enemy enemy = enemies.at(indexToRemove);

		//erase enemy from alive vector
		enemies.erase(enemies.begin() + indexToRemove);
		Texture* t_alive = enemy.t_alive;
		Texture* t_destroyed = enemy.t_destroyed;
		createEnemyRandomPosition(t_alive, t_destroyed);

		//insert enemy into destroyed vector
		enemy.sprite.setTexture(*enemy.t_destroyed);
		EnemyDestroyed destroyed(enemy.sprite);		
		enemies_destroyed.push_back(destroyed);
	}

	void update()
	{
		for(Enemy& enemy : enemies)
			enemy.update();

		for(EnemyDestroyed& enemy : enemies_destroyed)
			enemy.update();		

		for(unsigned int i = 0; i < enemies_destroyed.size(); i++)
		{
			EnemyDestroyed destroyed = enemies_destroyed.at(i);
			if(destroyed.mustDisappear())
			{
				enemies_destroyed.erase(enemies_destroyed.begin() + i);
				i--;
			}
		}
	}
};
/*----------------------EnemyManager----------------------*/

/*----------------------Player----------------------*/
struct Player
{
	Sprite sprite;
	Vector2f velocity;
	float defaultVelocity;

	Player(Texture* texture, float defaultVelocity, float mX, float mY) 
	{ 
		sprite.setTexture(*texture);
		sprite.setPosition(mX, mY);
		this->defaultVelocity = defaultVelocity;
	}
	
	void update()
	{
		Vector2u textureSize = sprite.getTexture()->getSize();

		sprite.setOrigin(textureSize.x/2, textureSize.y/2);

		velocity.x = 0;
		velocity.y = 0;

		Vector2f spritePosition = sprite.getPosition();

		if(Keyboard::isKeyPressed(Keyboard::Key::Left)) 
		{
			if(spritePosition.x - textureSize.x/2 > 0)
			{
				velocity.x = -defaultVelocity;
				velocity.y = 0;
			}
		}
		else if(Keyboard::isKeyPressed(Keyboard::Key::Right))
		{
			if(spritePosition.x + textureSize.x/2 < window_width)
			{
				velocity.x = defaultVelocity;
				velocity.y = 0;	
			}
		}

		if(Keyboard::isKeyPressed(Keyboard::Key::Down)) 
		{
			if(spritePosition.y + textureSize.y/2 < window_height)
			{
				velocity.x = 0;
				velocity.y = defaultVelocity;
			}
		}
		else if(Keyboard::isKeyPressed(Keyboard::Key::Up))
		{
			if(spritePosition.y - textureSize.y/2 > 0)
			{
				velocity.x = 0;
				velocity.y = -defaultVelocity;	
			}	
		}

		sprite.move(velocity);
	}

	Vector2f getDirection() //in realtà ora è inutile, ma la tengo per questioni di compatibilità col vecchio codice
	{
		Vector2f direction;
		direction.x = -1;
		direction.y = 0;

		return direction;
	}
};
/*----------------------Player----------------------*/

/*----------------------Bullet----------------------*/
struct Bullet
{
	Sprite sprite;
	Vector2f velocity;
	Vector2f direction;

	Bullet(Texture* texture, Player player)
	{
		sprite.setTexture(*texture);
		Vector2f position = player.sprite.getPosition();
		sprite.setPosition(position.x - player.sprite.getTexture()->getSize().x/2.f, position.y); //TODO usare la direction nel caso di rotazioni
		//sprite.setPosition(position.x, position.y);
		direction = player.getDirection();
	}

	void update()
	{
		velocity.x = direction.x*bulletVelocity;
		velocity.y = direction.y*bulletVelocity;

		sprite.move(velocity);
	}
};
/*----------------------Bullet----------------------*/

/*----------------------BulletManager----------------------*/
struct BulletManager
{
	vector<Bullet> bullets;
	Texture* texture;
	SoundBuffer* missile_sound_buffer;
	Sound missile_sound;
	Clock interval_bullets;
	float default_shot_after = 0.2f;
	float shoot_after = default_shot_after; //potrebbe essere specifico di proiettile in proiettile (power up)

	BulletManager(Texture* texture, SoundBuffer* missile_sound_buffer)
	{
		this->texture = texture;
		this->missile_sound_buffer = missile_sound_buffer;
		missile_sound.setBuffer(*missile_sound_buffer);
		missile_sound.setVolume(20);
	}

	void update(Player player)
	{
		if(interval_bullets.getElapsedTime().asSeconds() >= shoot_after)
		{
			if(Keyboard::isKeyPressed(Keyboard::Key::Space))		
			{
				bullets.emplace_back(texture, player);
				missile_sound.play();
			}	

			interval_bullets.restart();
		}
		
		bullets.erase(remove_if(begin(bullets), end(bullets), 
			[](const Bullet& bullet){
				Vector2f bPos = bullet.sprite.getPosition();
				return bPos.x > window_width || bPos.x < 0 || bPos.y > window_height || bPos.y < 0; 
			}), 
			end(bullets));

		for(Bullet& bullet : bullets)
			bullet.update();
	}
	
	void collision(int indexToRemove)
	{
		bullets.erase(bullets.begin() + indexToRemove);
	}
};
/*----------------------BulletManager----------------------*/

/*----------------------EnemyDestroyedCounter----------------------*/
struct EnemyDestroyedCounter
{
	Text text;
	int counter = 0;

	EnemyDestroyedCounter(Text text)
	{
		this->text = text;
		text.setString("0");
	}

	void increaseCounter()
	{
		counter++;
		char strCounter[50];
		sprintf(strCounter, "Kills: %d", counter);
		text.setString(strCounter);
	}
};
/*----------------------EnemyDestroyedCounter----------------------*/

/*----------------------LifeBar----------------------*/
struct LifeBar
{
	Sprite sprite;
	int max_life;
	int life;

	LifeBar(Texture* texture, int max_life)
	{
		sprite.setTexture(*texture);
		Vector2u textureSize = texture->getSize();

		this->max_life = max_life;
		life = max_life;

		sprite.setPosition(window_width - textureSize.x - 20, textureSize.y);
	}

	bool LifeDown()
	{
		life--;
		setLifeBar();

		return life == 0;
	}

	void LifeUp()
	{
		life++;
		setLifeBar();
	}

	void setLifeBar()
	{
		Vector2u textureSize = sprite.getTexture()->getSize();
		IntRect rect(0, 0, (textureSize.x/max_life)*life, textureSize.y);
		sprite.setTextureRect(rect);
	}
};
/*----------------------LifeBar----------------------*/

/*----------------------PowerUp----------------------*/
struct PowerUp
{
	Sprite sprite;
	Vector2f velocity;
	float probPerCent;
	Clock duration;
	float end_after;
	int power_up_type;	

	//define power up type
	PowerUp(Texture* texture, Vector2f velocity, float end_after, float probPerCent, int power_up_type)
	{
		sprite.setTexture(*texture);
		this->velocity = velocity;
		this->probPerCent = probPerCent;
		this->end_after = end_after;
		this->power_up_type = power_up_type;
	}

	PowerUp(Texture* texture, Vector2f velocity, float end_after, float probPerCent, int power_up_type, float mX, float mY)
	{
		sprite.setTexture(*texture);
		sprite.setPosition(mX, mY);
		this->velocity = velocity;
		this->probPerCent = probPerCent;
		this->end_after = end_after;
		this->power_up_type = power_up_type;
	}

	bool update()
	{
		sprite.move(velocity);
		return (sprite.getPosition().x >= window_width);
	}

	//sarebbe più pulito farlo con le classi derivate
	//virtual void powerUp(Player& player) = 0;
	//virtual void powerDown(Player& player) = 0;

	//in caso di classi derivate, questa funzione dovrebbe essere astratta e virtuale
	void powerUp(Player& player, LifeBar& lifeBar, BulletManager& bulletManager, DoubleBackground& doubleBackground)
	{
		duration.restart(); //avvia il timer dopo che il giocatore ha ottenuto il power up

		if(power_up_type == 1)
		{
			player.defaultVelocity *= 2;
			doubleBackground.move_x = doubleBackground.default_move_x*2;
		}
		else if(power_up_type == 2)
		{
			if(lifeBar.life < lifeBar.max_life)
				lifeBar.LifeUp();
		}
		else if(power_up_type == 3)
		{
			bulletManager.shoot_after = 0.1f;
		}
	}

	//in caso di classi derivate, questa funzione dovrebbe essere astratta e virtuale
	void powerDown(Player& player, BulletManager& bulletManager, DoubleBackground& doubleBackground)
	{
		if(power_up_type == 1)
		{
			player.defaultVelocity /= 2;
			doubleBackground.move_x = doubleBackground.default_move_x;
		}
		else if(power_up_type == 3)
		{
			bulletManager.shoot_after = bulletManager.default_shot_after;
		}
	}
	
	bool expired()
	{
		return duration.getElapsedTime().asSeconds() >= end_after;
	}
};
/*----------------------PowerUp----------------------*/

/*----------------------PowerUpManager----------------------*/
struct PowerUpManager
{
	vector<PowerUp> power_up_types;
	vector<PowerUp> power_ups;
	vector<PowerUp> capturedByPlayer;
	Clock clock_pu;
	float pick_every = 5.f;

	PowerUpManager()
	{

	}

	void addPowerUpType(Texture* texture, Vector2f velocity, float end_after, float probPerCent, int power_up_type)
	{
		power_up_types.emplace_back(texture, velocity, end_after, probPerCent, power_up_type);
	}

	void update(Player& player, BulletManager& bulletManager, DoubleBackground& doubleBackground)
	{
		if(clock_pu.getElapsedTime().asSeconds() >= pick_every)
		{
			if(power_up_types.size() > 0)
			{
				int randIndex = (int)(rand()%power_up_types.size()); //estraggo casualmente un tipo di powerup
				PowerUp powerupType = power_up_types.at(randIndex);

				int prob = (int)(rand()%100);
				if(prob <= powerupType.probPerCent) //genero randomicamente un power up di quel tipo
				{
					Vector2u textureSize = powerupType.sprite.getTexture()->getSize();
					int height = (int)(rand()%(window_height - textureSize.y*2) + textureSize.y*2); //lo posiziono casualmente
					Texture* texture = 	(Texture*)powerupType.sprite.getTexture();
					power_ups.emplace_back(texture, powerupType.velocity, powerupType.end_after, powerupType.probPerCent, powerupType.power_up_type, textureSize.x, height);
					
					clock_pu.restart(); //resetto il clock solo se sono riuscito ad estrarre un power up
				}
			}
		}

		for(unsigned int i = 0; i < power_ups.size(); i++)
    	{    		
    		PowerUp& powerup = power_ups.at(i);
    		if(powerup.update())
    		{
    			power_ups.erase(power_ups.begin() + i);
    			i--;
    		}
    	}

    	for(unsigned int i = 0; i < capturedByPlayer.size(); i++)
    	{
    		PowerUp& powerup = capturedByPlayer.at(i);    		
    		if(powerup.expired())
    		{
    			powerup.powerDown(player, bulletManager, doubleBackground);
    			capturedByPlayer.erase(capturedByPlayer.begin() + i);
    			i--;
    		}
    	}
	}

	void collision(Player& player, BulletManager& bulletManager, LifeBar& lifeBar, DoubleBackground& doubleBackground, int index)
	{
		PowerUp& powerUp = power_ups.at(index);
		powerUp.powerUp(player, lifeBar, bulletManager, doubleBackground);

		power_ups.erase(power_ups.begin() + index);

		capturedByPlayer.push_back(powerUp);
	}
};
/*----------------------PowerUpManager----------------------*/

/*----------------------main----------------------*/

int main()
{
    RenderWindow window(VideoMode(window_width, window_height), "Dummy Shooter!"/*, Style::Fullscreen*/);
    window.setFramerateLimit(60);

    srand(time(NULL));

    /*Background*/
	Texture background_texture_1;
    if(!background_texture_1.loadFromFile("resources/sky.png", IntRect(0, 0, window_width, window_height)))
    	return EXIT_FAILURE;

	Texture background_texture_2;
    if(!background_texture_2.loadFromFile("resources/sky-flipped.png", IntRect(0, 0, window_width, window_height)))
    	return EXIT_FAILURE;

    DoubleBackground doubleBackground(&background_texture_1, &background_texture_2);
	/*Background*/

    /*Player*/
	Texture player_texture;
    if (!player_texture.loadFromFile("resources/player.png"))
        return EXIT_FAILURE;

    Vector2u playerTextureSize = player_texture.getSize();
    //Player player(&player_texture, window_width/2 - playerTextureSize.x/2, window_height/2 - playerTextureSize.y/2); //center of screen
    Player player(&player_texture, playerVelocity, window_width - playerTextureSize.x/2, window_height/2 - playerTextureSize.y/2);
    /*Player*/

    /*EnemyManager*/
	Texture enemy_texture1;
	if (!enemy_texture1.loadFromFile("resources/enemy1.png"))
        return EXIT_FAILURE;

	Texture enemy_texture2;
	if (!enemy_texture2.loadFromFile("resources/enemy2.png"))
        return EXIT_FAILURE;

    Texture enemy_texture3;
	if (!enemy_texture3.loadFromFile("resources/enemy3.png"))
        return EXIT_FAILURE;

    Texture enemy_destroyed;
	if (!enemy_destroyed.loadFromFile("resources/explosion.png"))
        return EXIT_FAILURE;

    EnemyManager enemyManager;
    enemyManager.createEnemyRandomPosition(&enemy_texture1, &enemy_destroyed);
    enemyManager.createEnemyRandomPosition(&enemy_texture2, &enemy_destroyed);
    enemyManager.createEnemyRandomPosition(&enemy_texture3, &enemy_destroyed);
    /*EnemyManager*/

    /*BulletManager*/
	Texture bullets_texture;
	if (!bullets_texture.loadFromFile("resources/missile.png"))
        return EXIT_FAILURE;

	SoundBuffer missile_sound_buffer;
		if (!missile_sound_buffer.loadFromFile("resources/missile_launch.wav"))
	        return -1;

    BulletManager bulletManager(&bullets_texture, &missile_sound_buffer);
    /*BulletManager*/

    /*PowerUpManager*/
    Texture power_up_1_texture;
	if (!power_up_1_texture.loadFromFile("resources/power-up.png"))
        return EXIT_FAILURE;

    Texture power_up_2_texture;
	if (!power_up_2_texture.loadFromFile("resources/life-up.png"))
        return EXIT_FAILURE;

    Texture power_up_3_texture;
	if (!power_up_3_texture.loadFromFile("resources/missile_power_up.png"))
        return EXIT_FAILURE;

    PowerUpManager powerUpManager;    

	Vector2f pu1Velocity(10.f, 0.f);
	float pu1EndAfter{5.f};
	float pu1ProbPerCent{30.f};	
	int puType1 = 1;

	Vector2f pu2Velocity(10.f, 0.f);
	float pu2EndAfter{5.f};	
	float pu2ProbPerCent{20.f}; //100 per farlo apparire sicuramente ogni tot secondi
	int puType2 = 2; 

	Vector2f pu3Velocity(10.f, 0.f);
	float pu3EndAfter{5.f};	
	float pu3ProbPerCent{30.f}; //100 per farlo apparire sicuramente ogni tot secondi
	int puType3 = 3; 
      
    powerUpManager.addPowerUpType(&power_up_1_texture, pu1Velocity, pu1EndAfter, pu1ProbPerCent, puType1);      
	powerUpManager.addPowerUpType(&power_up_2_texture, pu2Velocity, pu2EndAfter, pu2ProbPerCent, puType2);
	powerUpManager.addPowerUpType(&power_up_3_texture, pu3Velocity, pu3EndAfter, pu3ProbPerCent, puType3);
    /*PowerUpManager*/

    /*TextInformation*/
    Text textInfo;
	Font fontTextInfo;
	if (!fontTextInfo.loadFromFile("resources/FreeSansBoldOblique.ttf"))			
	    return EXIT_FAILURE;

	textInfo.setFont(fontTextInfo);
	textInfo.setString("Kills: 0");
	textInfo.setCharacterSize(20);
	textInfo.setStyle(Text::Bold);
	textInfo.setColor(Color::Red);
	textInfo.setPosition(window_width - 100, 30);

    EnemyDestroyedCounter enemyCounter(textInfo);
    /*TextInformation*/

	/*Life*/
    Texture life_texture;
	if (!life_texture.loadFromFile("resources/life.png"))
        return EXIT_FAILURE;

    int max_life = 4; //la texture è costruita per 4 vite
    LifeBar lifeBar(&life_texture, max_life);
	/*Life*/

	/*Music*/
	//Music music;
	//if (!music.openFromFile("resources/Kavinsky-1986.ogg"))
	//	return EXIT_FAILURE;

	//music.setLoop(true);
	//music.play();
    /*Music*/

    /*Explosion*/
	SoundBuffer explosion_sound_buffer;
		if (!explosion_sound_buffer.loadFromFile("resources/explosion.wav"))
	        return -1;
	Sound explosion_sound;
	explosion_sound.setBuffer(explosion_sound_buffer);
	explosion_sound.setVolume(30);
    /*Explosion*/

	//View view;
	//view.reset(FloatRect(0, 0, window_width, window_height));
	//window.setView(view);
	// Initialize the view to a rectangle located at (100, 100) and with a size of 400x200
	//view.reset(sf::FloatRect(0, 0, 1920, 1080));
	// Set its target viewport to be half of the window
	//view.setViewport(sf::FloatRect(0.f, 0.f, 0.5f, 1.f));	

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Key::Escape))
                window.close();
        }

    	if(gameover)
    	{
			Text text;
			const char* game_over = "GAME OVER";
			int char_size = 50;			

			Font font;
			if (!font.loadFromFile("resources/FreeSansBoldOblique.ttf"))			
			    return EXIT_FAILURE;			

			text.setFont(font);
			text.setString(game_over);
			text.setCharacterSize(char_size);
			text.setStyle(Text::Bold);
			text.setColor(Color::Red);

			FloatRect boundingRect = text.getLocalBounds();

			text.setPosition(window_width/2 - boundingRect.width/2, window_height/2 - boundingRect.height/2);
			window.draw(text);

    		window.display();
    		continue;
    	}    	

    	window.clear();

        player.update();
        enemyManager.update();
        bulletManager.update(player);
		powerUpManager.update(player, bulletManager, doubleBackground);
		doubleBackground.update();

        /*window draw calls*/
        window.draw(doubleBackground.background_1);
        window.draw(doubleBackground.background_2);
        window.draw(player.sprite);

        for(Bullet& bullet : bulletManager.bullets)
        	window.draw(bullet.sprite);

        for(Enemy& enemy : enemyManager.enemies)
        	window.draw(enemy.sprite);

      	for(EnemyDestroyed& enemy : enemyManager.enemies_destroyed)
    		window.draw(enemy.sprite);

    	for(PowerUp& pu : powerUpManager.power_ups)
    		window.draw(pu.sprite);

    	//window.draw(lifeBar.sprite); la richiamo a fine loop, così quando arriva il gameover la barra si azzera e non rimane a uno

    	window.draw(enemyCounter.text);
    	/*window draw calls*/

		/*
		bulletManager.bullets.erase(remove_if(begin(bulletManager.bullets), end(bulletManager.bullets), 
		[player](const Bullet& bullet){ return Collision::CircleTest(player.sprite, bullet.sprite); }), 
		end(bulletManager.bullets));
		*/

		/*collision*/
		//collisione enemy-bullet
        for(unsigned int i = 0; i < enemyManager.enemies.size(); i++)
        {	
        	Enemy& enemy = enemyManager.enemies.at(i);
        	bool haveCollided = false;

        	for(unsigned int j = 0; j < bulletManager.bullets.size(); j++)
        	{	        	
        		Bullet& bullet = bulletManager.bullets.at(j);

        		if(Collision::CircleTest(bullet.sprite, enemy.sprite)) //BoundingBoxTest 
        		{
        			bulletManager.bullets.erase(bulletManager.bullets.begin() + j);
        			haveCollided = true;
        			break;
        		}
        	}

        	if(haveCollided)    	   	
        	{
        		explosion_sound.play();
        		enemyCounter.increaseCounter();
        		enemyManager.collision(i);
        		i--;
        		continue;
        	}
		}

        //collisione player-enemy
        for(unsigned int i = 0; i < enemyManager.enemies.size(); i++)
        {	
        	Enemy& enemy = enemyManager.enemies.at(i);
        	
        	if(Collision::CircleTest(player.sprite, enemy.sprite))  //BoundingBoxTest       	
        	{
        		if(lifeBar.LifeDown())
        			gameover = true;

        		explosion_sound.play();
        		enemyCounter.increaseCounter();
        		enemyManager.collision(i);
        		i--;
        	}
		}

		//collisione player-powerup
        for(unsigned int i = 0; i < powerUpManager.power_ups.size(); i++)
        {	
        	PowerUp& pu = powerUpManager.power_ups.at(i);
        	
        	if(Collision::CircleTest(player.sprite, pu.sprite))  //BoundingBoxTest       	
        	{
        		powerUpManager.collision(player, bulletManager, lifeBar, doubleBackground, i);
        		i--;
        	}
		}
		/*collision*/

		/*window draw*/
		window.draw(lifeBar.sprite);
		/*window draw*/

        window.display();
    }



    return 0;
}
