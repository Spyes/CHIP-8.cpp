#include <memory>
#include <stdio.h>
#include "raylib.h"
#include "cpu.h"
#include "compiler.h"

const uint8_t SCREEN_SCALE = 16;

void DrawDebug();

int main(void)
{
	std::unique_ptr<Compiler> compiler = std::make_unique<Compiler>();
	compiler->Compile("./roms/test.asm");

	std::unique_ptr<CPU> chip8 = std::make_unique<CPU>();
	chip8->Initialize();
	chip8->LoadProgram(compiler->Generator());
	// chip8->ReadROM("./roms/1-ibm-logo.ch8");

	InitWindow(SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, "CHIP-8");
	SetTargetFPS(60);

	bool isDebug = false;

	while (!WindowShouldClose())
	{
		if (IsKeyPressed(KEY_F1)) { isDebug = !isDebug; }

		if        (IsKeyPressed(KEY_ONE)) {
			chip8->SetKey(0x00);
		} else if (IsKeyPressed(KEY_TWO)) {
			chip8->SetKey(0x01);
		} else if (IsKeyPressed(KEY_THREE)) {
			chip8->SetKey(0x02);
		} else if (IsKeyPressed(KEY_FOUR)) {
			chip8->SetKey(0x03);
		} else if (IsKeyPressed(KEY_Q)) {
			chip8->SetKey(0x04);
		} else if (IsKeyPressed(KEY_W)) {
			chip8->SetKey(0x05);
		} else if (IsKeyPressed(KEY_E)) {
			chip8->SetKey(0x06);
		} else if (IsKeyPressed(KEY_R)) {
			chip8->SetKey(0x07);
		} else if (IsKeyPressed(KEY_A)) {
			chip8->SetKey(0x08);
		} else if (IsKeyPressed(KEY_S)) {
			chip8->SetKey(0x09);
		} else if (IsKeyPressed(KEY_D)) {
			chip8->SetKey(0x0A);
		} else if (IsKeyPressed(KEY_F)) {
			chip8->SetKey(0x0B);
		} else if (IsKeyPressed(KEY_Z)) {
			chip8->SetKey(0x0C);
		} else if (IsKeyPressed(KEY_X)) {
			chip8->SetKey(0x0D);
		} else if (IsKeyPressed(KEY_C)) {
			chip8->SetKey(0x0E);
		} else if (IsKeyPressed(KEY_V)) {
			chip8->SetKey(0x0F);
		}

		if        (IsKeyReleased(KEY_ONE)) {
			chip8->UnsetKey(0x00);
		} else if (IsKeyReleased(KEY_TWO)) {
			chip8->UnsetKey(0x01);
		} else if (IsKeyReleased(KEY_THREE)) {
			chip8->UnsetKey(0x02);
		} else if (IsKeyReleased(KEY_FOUR)) {
			chip8->UnsetKey(0x03);
		} else if (IsKeyReleased(KEY_Q)) {
			chip8->UnsetKey(0x04);
		} else if (IsKeyReleased(KEY_W)) {
			chip8->UnsetKey(0x05);
		} else if (IsKeyReleased(KEY_E)) {
			chip8->UnsetKey(0x06);
		} else if (IsKeyReleased(KEY_R)) {
			chip8->UnsetKey(0x07);
		} else if (IsKeyReleased(KEY_A)) {
			chip8->UnsetKey(0x08);
		} else if (IsKeyReleased(KEY_S)) {
			chip8->UnsetKey(0x09);
		} else if (IsKeyReleased(KEY_D)) {
			chip8->UnsetKey(0x0A);
		} else if (IsKeyReleased(KEY_F)) {
			chip8->UnsetKey(0x0B);
		} else if (IsKeyReleased(KEY_Z)) {
			chip8->UnsetKey(0x0C);
		} else if (IsKeyReleased(KEY_X)) {
			chip8->UnsetKey(0x0D);
		} else if (IsKeyReleased(KEY_C)) {
			chip8->UnsetKey(0x0E);
		} else if (IsKeyReleased(KEY_V)) {
			chip8->UnsetKey(0x0F);
		}

		chip8->DecrementTimers();

		chip8->Fetch();
		chip8->Decode();

		BeginDrawing();
			ClearBackground(BLACK);

			DrawRectangle(0, 0, SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, DARKGRAY);
			for (uint16_t p = 0; p < chip8->GetDisplay().size(); p++) {
				uint16_t x = p % SCREEN_WIDTH;
				uint16_t y = p / SCREEN_WIDTH;
				if (chip8->GetDisplay().at(p)) {
					DrawRectangle(x * SCREEN_SCALE, y * SCREEN_SCALE, SCREEN_SCALE, SCREEN_SCALE, LIME);
				}
				if (isDebug) {
					DrawRectangleLines(x * SCREEN_SCALE, y * SCREEN_SCALE, SCREEN_SCALE, SCREEN_SCALE, BLACK);
				}
			}

		EndDrawing();
	}

	CloseWindow();

	return 0;
}

void DrawDebug() {
	for (uint8_t row = 0; row < SCREEN_HEIGHT; row++) {
		DrawLine(0, row * SCREEN_SCALE, SCREEN_WIDTH * SCREEN_SCALE, row * SCREEN_SCALE, BLACK);
	}

	for (uint8_t col = 0; col < SCREEN_WIDTH; col++) {
		DrawLine(col * SCREEN_SCALE, 0, col * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, BLACK);
	}
}