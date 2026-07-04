import pygame
import sys
import random

pygame.init()

# Screen dimensions
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Pong with Score and Best of 9")

# Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)

# Paddle settings
PADDLE_WIDTH, PADDLE_HEIGHT = 10, 100
PADDLE_SPEED = 5
AI_PADDLE_SPEED = 3  # Slightly slower AI for balanced gameplay

# Ball settings
BALL_SIZE = 10
BALL_SPEED_X = 4
BALL_SPEED_Y = 4

# Initialize paddles and ball
player_paddle = pygame.Rect(10, HEIGHT // 2 - PADDLE_HEIGHT // 2, PADDLE_WIDTH, PADDLE_HEIGHT)
ai_paddle = pygame.Rect(WIDTH - PADDLE_WIDTH - 10, HEIGHT // 2 - PADDLE_HEIGHT // 2, PADDLE_WIDTH, PADDLE_HEIGHT)
ball = pygame.Rect(WIDTH // 2 - BALL_SIZE // 2, HEIGHT // 2 - BALL_SIZE // 2, BALL_SIZE, BALL_SIZE)

# Initialize ball velocity
ball_vel_x, ball_vel_y = BALL_SPEED_X, BALL_SPEED_Y

# Initialize clock
clock = pygame.time.Clock()

# Scores and turns
player_score = 0
ai_score = 0
player_turns_won = 0
ai_turns_won = 0
max_turns = 9  # total turns in the match
turns_played = 0

# Font for displaying scores and messages
font = pygame.font.SysFont(None, 36)

def reset_ball(current_vel_x):
    """Reset ball to center with reversed horizontal direction."""
    ball.center = (WIDTH // 2, HEIGHT // 2)
    ball_vel_x = -current_vel_x  # Reverse direction
    ball_vel_y = BALL_SPEED_Y * (1 if random.random() < 0.5 else -1)  # Randomize vertical direction
    return ball_vel_x, ball_vel_y

def draw_scores():
    """Draw current scores at the top center."""
    score_text = f"Player: {player_score}  AI: {ai_score}"
    text_surface = font.render(score_text, True, WHITE)
    screen.blit(text_surface, (WIDTH // 2 - text_surface.get_width() // 2, 10))

def check_winner():
    """Determine if game has a winner or if max turns reached."""
    global player_score, ai_score, turns_played
    if player_score >= 5:
        return "Player wins!"
    elif ai_score >= 5:
        return "AI wins!"
    elif turns_played >= max_turns:
        if player_score > ai_score:
            return "Player wins!"
        elif ai_score > player_score:
            return "AI wins!"
        else:
            return "It's a tie!"
    return None

# Main game loop
game_over = False
while not game_over:
    dt = clock.tick(60) / 1000  # Time delta for smooth movement

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()

    # Player control
    keys = pygame.key.get_pressed()
    if keys[pygame.K_w] and player_paddle.top > 0:
        player_paddle.y -= PADDLE_SPEED
    if keys[pygame.K_s] and player_paddle.bottom < HEIGHT:
        player_paddle.y += PADDLE_SPEED

    # Simple AI movement
    if ai_paddle.centery < ball.centery and ai_paddle.bottom < HEIGHT:
        ai_paddle.y += min(AI_PADDLE_SPEED, ball.centery - ai_paddle.centery)
    elif ai_paddle.centery > ball.centery and ai_paddle.top > 0:
        ai_paddle.y -= min(AI_PADDLE_SPEED, ai_paddle.centery - ball.centery)

    # Move the ball
    ball.x += ball_vel_x
    ball.y += ball_vel_y

    # Collisions with top and bottom walls
    if ball.top <= 0 or ball.bottom >= HEIGHT:
        ball_vel_y *= -1

    # Collisions with paddles
    if ball.colliderect(player_paddle) and ball_vel_x < 0:
        ball_vel_x *= -1
    if ball.colliderect(ai_paddle) and ball_vel_x > 0:
        ball_vel_x *= -1

    # Scoring
    if ball.left <= 0:
        # AI scores
        ai_score += 1
        turns_played += 1
        ball_vel_x, ball_vel_y = reset_ball(ball_vel_x)
    elif ball.right >= WIDTH:
        # Player scores
        player_score += 1
        turns_played += 1
        ball_vel_x, ball_vel_y = reset_ball(ball_vel_x)

    # Check for game end
    winner_text = check_winner()
    if winner_text:
        game_over = True

    # Draw everything
    screen.fill(BLACK)
    pygame.draw.rect(screen, WHITE, player_paddle)
    pygame.draw.rect(screen, WHITE, ai_paddle)
    pygame.draw.ellipse(screen, WHITE, ball)
    draw_scores()

    if game_over:
        # Display the winner message
        end_text = font.render(winner_text, True, WHITE)
        screen.blit(end_text, (WIDTH // 2 - end_text.get_width() // 2, HEIGHT // 2))
        pygame.display.flip()
        pygame.time.wait(3000)  # Wait 3 seconds before quitting
        pygame.quit()
        sys.exit()

    pygame.display.flip()
