#include "shape.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "baseTypes.h"

const float DEG2RAD = 3.14159f/180.0f;

/// @brief Draws a circle to the screen w/ the given properties
/// @param radius 
/// @param x center X
/// @param y center Y
/// @param r red
/// @param g green
/// @param b blue
/// @param filled solid circle, if true, outline otherwise
void shapeDrawCircle(float radius, float x, float y, uint8_t r, uint8_t g, uint8_t b, bool filled)
{	
	glEnable(GL_POINT_SMOOTH);
	glDisable(GL_TEXTURE_2D);
	if(!filled)
	{
		glColor4ub(r, g, b, 0xFF);
		// Set the point size
		glPointSize(1.0);
		glBegin(GL_POINTS);
		float radiusMax = radius + 2.0f;
		float radiusMin = radius - 2.0f;
		for (int i=0; i < 360; i+=3)
		{
			float degInRad = i*DEG2RAD;
			glVertex2f(x + (cos(degInRad) * radius), y + (sin(degInRad) * radius));
			glVertex2f(x + (cos(degInRad) * radiusMax), y + (sin(degInRad) * radiusMax));
			glVertex2f(x + (cos(degInRad) * radiusMin), y + (sin(degInRad) * radiusMin));
		}
		glEnd();
	}
	else
	{
	    glColor4ub(r, g, b, 0xFF);
		glEnable(GL_BLEND); 
	    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
		glPointSize(radius * 2);
		glBegin(GL_POINTS);
		glVertex3f(x, y, 1);
		glEnd();
	}
}

/// @brief Draws a line to the screen with the given properties
/// @param startX 
/// @param startY 
/// @param endX 
/// @param endY 
/// @param r red
/// @param g green
/// @param b blue
/// @param width
void shapeDrawLine(float startX, float startY, float endX, float endY, uint8_t r, uint8_t g, uint8_t b, GLfloat width)
{
	glLineWidth(width);
	glColor3ub(r, g, b);
	// Draw filtered lines
	glEnable(GL_LINE_SMOOTH);
	glDisable(GL_TEXTURE_2D);

	glBegin(GL_LINE_STRIP);
		glVertex3f(startX, startY, 1.0f);
		glVertex3f(endX, endY, 1.0f);
	glEnd();
	glLineWidth(1);
	glColor3ub(255, 255, 255);

}

void shapeDrawTrapezoid(ShapeTrapezoid* shape, uint8_t r, uint8_t g, uint8_t b)
{
	shapeDrawLine(shape->a.x, shape->a.y, shape->b.x, shape->b.y, r, g, b, 1);
	shapeDrawLine(shape->b.x, shape->b.y, shape->c.x, shape->c.y, r, g, b, 1);
	shapeDrawLine(shape->c.x, shape->c.y, shape->d.x, shape->d.y, r, g, b, 1);
	shapeDrawLine(shape->d.x, shape->d.y, shape->a.x, shape->a.y, r, g, b, 1);
}

void shapeDrawRect(Vector2 topLeft, Vector2 botRight, uint8_t r, uint8_t g, uint8_t b, GLfloat width, bool filled)
{
	if( filled)
	{
		glDisable(GL_DEPTH_TEST); 
		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_TEXTURE_2D);

		glColor3ub(r, g, b);
		glBegin(GL_QUADS);

		glVertex3f(topLeft.x, botRight.y, 1.0f);
		glVertex3f(botRight.x, botRight.y, 1.0f);
		glVertex3f(botRight.x, topLeft.y, 1.0f);
		glVertex3f(topLeft.x, topLeft.y, 1.0f);

		glEnd();

		glEnable(GL_TEXTURE_2D);
		glColor3ub(255, 255, 255);
	}
	else
	{
		shapeDrawLine(topLeft.x, topLeft.y, botRight.x, topLeft.y, r, g, b, width); // Top edge
		shapeDrawLine(topLeft.x, botRight.y, botRight.x, botRight.y, r, g, b, width); // Bottom edge
		shapeDrawLine(topLeft.x, topLeft.y, topLeft.x, botRight.y, r, g, b, width); // Left edge
		shapeDrawLine(botRight.x, topLeft.y, botRight.x, botRight.y, r, g, b, width); // Right edge
	}
}

void shapeMove(Shape* shape, Vector2 transform)
{
	assert(shape != NULL);
	switch (shape->type)
	{
	case SHAPE_POINT:
		shape->point->position.x += transform.x;
		shape->point->position.y += transform.y;
		break;

	case SHAPE_CIRCLE:
		shape->circle->center.x += transform.x;
		shape->circle->center.y += transform.y;
		break;

	case SHAPE_RECTANGLE:
		shape->rectangle->position.x += transform.x;
		shape->rectangle->position.y += transform.y;
		break;

	case SHAPE_TRAPEZOID:
		shape->trapezoid->a = vector2Add(shape->trapezoid->a, transform);
		shape->trapezoid->b = vector2Add(shape->trapezoid->b, transform);
		shape->trapezoid->c = vector2Add(shape->trapezoid->c, transform);
		shape->trapezoid->d = vector2Add(shape->trapezoid->d, transform);

		break;
	}
}

void shapeSetPosition(Shape* shape, Vector2 position)
{
	assert(shape != NULL);
	switch (shape->type)
	{
	case SHAPE_POINT:
		shape->point->position.x = position.x;
		shape->point->position.y = position.y;
		break;

	case SHAPE_CIRCLE:
		shape->circle->center.x = position.x;
		shape->circle->center.y = position.y;
		break;

	case SHAPE_RECTANGLE:
		shape->rectangle->position.x = position.x - (shape->rectangle->size.x / 2);
		shape->rectangle->position.y = position.y - (shape->rectangle->size.y / 2);
		break;

	case SHAPE_TRAPEZOID:
		Vector2 center;
		center.x = (shape->trapezoid->a.x + shape->trapezoid->b.x + shape->trapezoid->c.x + shape->trapezoid->d.x) / 4;
		center.y = (shape->trapezoid->a.y + shape->trapezoid->b.y + shape->trapezoid->c.y + shape->trapezoid->d.y) / 4;
		Vector2 newPosition = vector2Sub(position, center);
		shapeMove(shape, newPosition);
		break;

	default:
		break;
	}
}
