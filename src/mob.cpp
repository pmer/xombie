#include <err.h>
#include <math.h>
#include <stdlib.h>

#include "common.h"
#include "conf.h"
#include "engine.h"
#include "mob.h"
#include "random.h"
#include "range.h"
#include "str.h"
#include "xml.h"

Conf mobs("conf/mobs.conf");

/* newRandomMob
 *
 * Chooses a mob at random, based on chance-type data in mobs.conf.
 * Returns a new mob.
 */
Mob* newRandomMob()
{
	int count = mobs.getInt("Spawn", "Mob Count");
	int percent = randInt(1, 100);
	int chance = 0;

	for (int i = 1; i <= count; i++) {
		Str chanceStr(20);
		chanceStr += "Chance ";
		chanceStr += i;

		chance += mobs.getInt("Spawn", chanceStr);
		if (chance >= percent) {
			Str typeStr(10);
			typeStr += "Type ";
			typeStr += i;

			const char* type = mobs.getString("Spawn", typeStr);
			Mob* m = new Mob(type);
			return m;
		}
	}

	throw "Creating a non-existant mob not implimented yet";
}


Mob::Mob(const char* Type)
	: dead(false), dur(0)
{
	strcpy(type, Type);

	init(mobs.getString(type, "Graphic"));
	setSpeed(mobs.getInt(type, "Speed"));
	hp = mhp = randInt(mobs.getInt(type, "Min HP"), 
	                   mobs.getInt(type, "Max HP"));
	dmg = mobs.getInt(type, "Damage");

	// metazombie
	damagable = mobs.getBool(type, "Can be damaged");
	if (damagable)
		graphicDmg = mobs.getString(type, "Damaged graphic");

	generateSpawnPosition();
}

void Mob::generateSpawnPosition()
{
	int screenHeight = getEngine()->getHeight();
	int screenWidth  = getEngine()->getWidth();

	// set mob to 45° to compensate for size changes with rotations
	// 45° is the widest and highest it will ever get
	setAngle(M_PI_2 / 2);
	int x = 0, y = 0;

	// TODO: mobs can come from any angle
	switch (randInt(0, 3)) {
		// left
		case 0:
			x = -gfx->w - 5;
			y = randInt(100, screenHeight - 100);
			setAngle(randDouble() * M_PI_2 - 1.0 * M_PI / 4.0);
			break;
		// right
		case 1:
			x = screenWidth + gfx->w + 5;
			y = randInt(100, screenHeight - 100);
			setAngle(randDouble() * M_PI_2 + 3.0 * M_PI / 4.0);
			break;
		// top
		case 2:
			x = randInt(100, screenHeight - 100);
			y = -gfx->h - 5;
			setAngle(randDouble() * M_PI_2 + 5.0 * M_PI / 4.0);
			break;
		// bottom
		case 3:
			x = randInt(100, screenHeight - 100);
			y = screenHeight + gfx->h + 5;
			setAngle(randDouble() * M_PI_2 + 1.0 * M_PI / 4.0);
			break;
	}

	setLoc(x, y);
}

void Mob::update(int dt)
{
	// don't start AI until mobs have appeared on screen already
	if (!isCompletelyOnScreen()) {
		move(dt);
		return;
	}

	dur -= dt;
	if (dur < 0) {
		dur = randInt(500, 2000);
		dir = randInt(-1, 1);
	}

	double ang = getAngle() + 0.001 * dt * dir;

	while (ang < 0)
		ang += 2 * M_PI;
	while (2 * M_PI < ang)
		ang -= 2 * M_PI;

	setAngle(ang);
	move(dt);
	stayOnScreen();
}

void Mob::draw(SDL_Surface* screen)
{
	Sprite::draw(screen);
}

char* Mob::spawnItem()
{
	static bool shotgun = false;
	if (!strcmp(type, "metazombie") && !shotgun) {
		shotgun = true;
		return (char*)"shotgun";
	}
	return NULL;
}

void Mob::setHP(int HP)
{
	hp = HP;
	if (hp > mhp)
		hp = mhp;

	if (damagable && hp <= mhp / 2)
		Sprite::setGraphic(graphicDmg);

	if (hp <= 0)
		dead = true;
}

bool Mob::isDead()
{
	return dead;
}

int Mob::getHP()
{
	return hp;
}

int Mob::getMaxHP()
{
	return mhp;
}

int Mob::getDamage()
{
	return dmg;
}
