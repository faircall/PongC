#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL.h"

#define SCREENWIDTH 800
#define SCREENHEIGHT 600

typedef unsigned char byte;

//this could be packed into an int, it's worth noting
typedef struct {
    byte r;
    byte g;
    byte b;
    byte a;
} Color;

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    Vec2 position;
    int width;
    int height;
} Paddle;

typedef struct {
    Vec2 position;
    Vec2 heading;
} Ball;

void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h, Color color);

Color init_color(byte red, byte green, byte blue, byte alpha);

Vec2 init_vec2(float x, float y);

Paddle init_paddle(float x, float y, int width, int height);

Ball init_ball(float x, float y, float x_heading, float y_heading);

void move_ball(Ball *ball, float ball_speed, Paddle paddle_p1, Paddle paddle_p2, float dt);

void reset_ball(Ball *ball);

void move_paddle(Paddle *paddle, Ball ball, float dt);

float distance(float x, float y);

float float_min(float x, float y);

float float_max(float x, float y);

int main(int argc, char **argv)
{

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
	printf("didn't init SDL\n");
	return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (SDL_CreateWindowAndRenderer(SCREENWIDTH, SCREENHEIGHT, 0, &window, &renderer) != 0) {
	printf("didn't create window/renderer \n");
	return 1;
    }
    
    
    int game_running = 1;

    /////////////Game specific values/////////////
    Color white = {0xff, 0xff, 0xff, 0x00};
    Paddle paddle_p1 = init_paddle(0.0f, 20.0f, 10.0f, 50.0f);
    Paddle paddle_p2 = init_paddle(SCREENWIDTH-10.0f, 20.0f, 10.0f, 50.0f);
    Ball ball = init_ball(SCREENWIDTH/2, SCREENHEIGHT/2, -1.0f, 0.5f);
    int mouse_x;
    int mouse_y;
    float ball_speed = 300.0f;
    float dt;
    unsigned int current_time, time_elapsed, last_time = 0;
    
    while (game_running) {
	current_time = SDL_GetTicks();
	time_elapsed = current_time - last_time;
	dt = ((float)time_elapsed)/1000.0f;
	last_time = current_time;
	
	SDL_Event event;
	SDL_PollEvent(&event);
	//get the mouse position
	if (event.type == SDL_QUIT) {
	    game_running = 0;
	}

	SDL_GetMouseState(&mouse_x, &mouse_y);

	paddle_p1.position.y = mouse_y;
	
	move_ball(&ball, ball_speed, paddle_p1, paddle_p2, dt);
	move_paddle(&paddle_p2, ball, dt);
	SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0x00);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0x00);
	draw_rect(renderer, paddle_p1.position.x, paddle_p1.position.y, paddle_p1.width, paddle_p1.height, white);
	draw_rect(renderer, paddle_p2.position.x, paddle_p2.position.y, paddle_p2.width, paddle_p2.height, white);
	
	draw_rect(renderer, ball.position.x, ball.position.y, 5.0f, 5.0f, white);
	SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

void draw_rect(SDL_Renderer *renderer, int x, int y, int w, int h, Color color)
{
    SDL_Rect temp_rect;
    Color old_color;
    SDL_GetRenderDrawColor(renderer, &old_color.r, &old_color.g, &old_color.b, &old_color.a);
    temp_rect.x = x;
    temp_rect.y = y;
    temp_rect.w = w;
    temp_rect.h = h;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &temp_rect);
    SDL_SetRenderDrawColor(renderer, old_color.r, old_color.g, old_color.b, old_color.a);
    
}

Color init_color(byte red, byte green, byte blue, byte alpha)
{
    Color result;
    result.r = red;
    result.g = green;
    result.b = blue;
    result.a = alpha;

    return result;
}

Vec2 init_vec2(float x, float y)
{
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

Paddle init_paddle(float x, float y, int width, int height)
{
    Vec2 position = init_vec2(x, y);
    Paddle result;
    result.position = position;
    result.width = width;
    result.height = height;
    return result;
}

Ball init_ball(float x, float y, float x_heading, float y_heading)
{

    Vec2 position = init_vec2(x,y);
    Vec2 heading = init_vec2(x_heading, y_heading);
    Ball result;
    result.position = position;
    result.heading = heading;
    return result;
}

void move_ball(Ball *ball, float ball_speed, Paddle paddle_p1, Paddle paddle_p2, float dt)
{
    
    ball->position.x += ball_speed*ball->heading.x*dt;
    ball->position.y += ball_speed*ball->heading.y*dt;

    //reflect along y axis
    if (ball->position.y <=0 || ball->position.y >= SCREENHEIGHT) {
	ball->heading.y *= -1.0f;
    }
    //reflect along x axis and handle case of
    //paddle


    if (ball->position.x <= 0) {
	if (ball->position.y >= paddle_p1.position.y && ball->position.y <= (paddle_p1.position.y + paddle_p1.height)) {
	    float p1_center = paddle_p1.position.y + paddle_p1.height/2;
	    float p1_ball_offset = (ball->position.y - p1_center)/(paddle_p1.height/2); 
	    ball->heading.x *= -1.0f;
	    ball->heading.y = p1_ball_offset;
	} else {
	    reset_ball(ball);
	}
    }

    if (ball->position.x >= SCREENWIDTH) {
	if (ball->position.y >= paddle_p2.position.y && ball->position.y <= (paddle_p2.position.y + paddle_p2.height)) {
	    float p2_center = paddle_p2.position.y + paddle_p2.height/2;
	    float p2_ball_offset = (ball->position.y - p2_center)/(paddle_p2.height/2); 
	    ball->heading.x *= -1.0f;
	    ball->heading.y = p2_ball_offset;
	} else {
	    reset_ball(ball);
	}
    }

}

void reset_ball(Ball *ball)
{
    ball->position.x = SCREENWIDTH/2;
    ball->position.y = SCREENHEIGHT/2;
    ball->heading.x *= -1.0f;
}

void move_paddle(Paddle *paddle, Ball ball, float dt)
{
    
    float distance_to_ball_x = distance(ball.position.x, paddle->position.x);
    float distance_to_ball_y = distance(ball.position.y,paddle->position.y + paddle->height/2);
    float signed_distance_to_ball_y = ball.position.y - (paddle->position.y + paddle->height/2);
    float reflex_scale = float_max(distance_to_ball_x,1.0f)/SCREENWIDTH;//between 0,1
    float reflect_lerped = 1.0f - reflex_scale;
    float max_move_speed = 250.0f*reflect_lerped;

    float direction;

    if (signed_distance_to_ball_y > 0) {
	direction = 1.0f;
    } else if (signed_distance_to_ball_y < 0) {
	direction = -1.0f;
    } else {
	direction = 0.0f;
    }
    if (distance_to_ball_y > paddle->height/4) {
	paddle->position.y += direction*max_move_speed * dt;
    }
}

float float_min(float x, float y)
{
    if (x <= y) {
	return x;
    }
    return y;
}

float float_max(float x, float y)
{
    if (x >= y) {
	return x;
    }
    return y;
}

float distance(float x, float y)
{
    float result;
    result = sqrt((x-y)*(x-y));
    return result;
}
