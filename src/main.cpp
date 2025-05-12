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