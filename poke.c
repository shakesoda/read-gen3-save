// essentially all information from https://bulbapedia.bulbagarden.net/wiki/Save_data_structure_in_Generation_III
// and otherwise from poking about in a hex editor
#define _CRT_SECURE_NO_WARNINGS
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _MSC_VER
#include "mman.h"
#else
#include <sys/mman.h> 
#endif

static void check (int test, const char * message, ...) {
	if (test) {
		va_list args;
		va_start(args, message);
		vfprintf(stderr, message, args);
		va_end(args);
		fprintf(stderr, "\n");
		exit(EXIT_FAILURE);
	}
}

uint8_t poke_to_ascii(const uint8_t poke_char) {
	uint8_t out_char = 0x0;

	if (poke_char >= 0xBB && poke_char <= 0xD4) // A - Z
		out_char = poke_char - 0xBB + 0x41;
	else if (poke_char >= 0xD5 && poke_char <= 0xEE) // a - z
		out_char = poke_char - 0xD5 + 0x61;
	else if (poke_char >= 0xA1 && poke_char <= 0xAA) // 0 - 9
		out_char = poke_char - 0xA1 + 0x30;
	else if (poke_char == 0xAB) // !
		out_char = 0x21;
	else if (poke_char == 0xAC) // ?
		out_char = 0x3F;
	else if (poke_char == 0xAD) // .
		out_char = 0x2E;
	else if (poke_char == 0xAE) // -
		out_char = 0x2D;
	else if (poke_char == 0xB1) // "
		out_char = 0x22;
	else if (poke_char == 0xB2) // "
		out_char = 0x22;
	else if (poke_char == 0xB3) // '
		out_char = 0x27;
	else if (poke_char == 0xB4) // '
		out_char = 0x27;
	else if (poke_char == 0x00) // space
		out_char = 0x20;

	return out_char;
}

// make sure to reserve len+1 for null terminator
void decode_text(uint8_t *ascii_out, const uint8_t *base, const uint8_t len) {
	for (int i = 0; i < len; i++) {
		ascii_out[i] = poke_to_ascii(base[i]);
	}
	ascii_out[len] = 0;
}

void dump_trainer_info(const uint8_t *base) {
	printf("\n\n");
	uint8_t name[8];
	decode_text(name, base, 7);
	printf("%s\n", name);

	bool female = base[0x8] == 0x1;
	printf("female: %d\n", female);

	uint32_t trainer_id = base[0xA];
	printf("trainer id %d\n", trainer_id);
	printf("\n\n");
}

enum poke_type {
	Normal,
	Water,
	Poison,
	Fire,
	Flying,
	Bug,
	Electric,
	Grass,
	Fighting,
	Steel,
	Rock,
	Ghost,
	Ground,
	Ice,
	Dragon,
	Dark,
	Psychic,
	Egg,
};

struct Pokemon {
	uint16_t hex;
	const char *name;
	enum poke_type type1;
	enum poke_type type2;
};

const struct Pokemon pokemon_lut[] = {
	{.hex = 0, .name = "?????????", .type1 = Normal, .type2 = Normal },
	{.hex = 1, .name = "Bulbasaur", .type1 = Grass, .type2 = Poison },
	{.hex = 2, .name = "Ivysaur", .type1 = Grass, .type2 = Poison },
	{.hex = 3, .name = "Venusaur", .type1 = Grass, .type2 = Poison },
	{.hex = 4, .name = "Charmander", .type1 = Fire, .type2 = Fire },
	{.hex = 5, .name = "Charmeleon", .type1 = Fire, .type2 = Fire },
	{.hex = 6, .name = "Charizard", .type1 = Fire, .type2 = Flying },
	{.hex = 7, .name = "Squirtle", .type1 = Water, .type2 = Water },
	{.hex = 8, .name = "Wartortle", .type1 = Water, .type2 = Water },
	{.hex = 9, .name = "Blastoise", .type1 = Water, .type2 = Water },
	{.hex = 10, .name = "Caterpie", .type1 = Bug, .type2 = Bug },
	{.hex = 11, .name = "Metapod", .type1 = Bug, .type2 = Bug },
	{.hex = 12, .name = "Butterfree", .type1 = Bug, .type2 = Flying },
	{.hex = 13, .name = "Weedle", .type1 = Bug, .type2 = Poison },
	{.hex = 14, .name = "Kakuna", .type1 = Bug, .type2 = Poison },
	{.hex = 15, .name = "Beedrill", .type1 = Bug, .type2 = Poison },
	{.hex = 16, .name = "Pidgey", .type1 = Normal, .type2 = Flying },
	{.hex = 17, .name = "Pidgeotto", .type1 = Normal, .type2 = Flying },
	{.hex = 18, .name = "Pidgeot", .type1 = Normal, .type2 = Flying },
	{.hex = 19, .name = "Rattata", .type1 = Normal, .type2 = Normal },
	{.hex = 20, .name = "Raticate", .type1 = Normal, .type2 = Normal },
	{.hex = 21, .name = "Spearow", .type1 = Normal, .type2 = Flying },
	{.hex = 22, .name = "Fearow", .type1 = Normal, .type2 = Flying },
	{.hex = 23, .name = "Ekans", .type1 = Poison, .type2 = Poison },
	{.hex = 24, .name = "Arbok", .type1 = Poison, .type2 = Poison },
	{.hex = 25, .name = "Pikachu", .type1 = Electric, .type2 = Electric },
	{.hex = 26, .name = "Raichu", .type1 = Electric, .type2 = Electric },
	{.hex = 27, .name = "Sandshrew", .type1 = Ground, .type2 = Ground },
	{.hex = 28, .name = "Sandslash", .type1 = Ground, .type2 = Ground },
	{.hex = 29, .name = "Nidoran?", .type1 = Poison, .type2 = Poison },
	{.hex = 30, .name = "Nidorina", .type1 = Poison, .type2 = Poison },
	{.hex = 31, .name = "Nidoqueen", .type1 = Poison, .type2 = Ground },
	{.hex = 32, .name = "Nidoran?", .type1 = Poison, .type2 = Poison },
	{.hex = 33, .name = "Nidorino", .type1 = Poison, .type2 = Poison },
	{.hex = 34, .name = "Nidoking", .type1 = Poison, .type2 = Ground },
	{.hex = 35, .name = "Clefairy", .type1 = Normal, .type2 = Normal },
	{.hex = 36, .name = "Clefable", .type1 = Normal, .type2 = Normal },
	{.hex = 37, .name = "Vulpix", .type1 = Fire, .type2 = Fire },
	{.hex = 38, .name = "Ninetales", .type1 = Fire, .type2 = Fire },
	{.hex = 39, .name = "Jigglypuff", .type1 = Normal, .type2 = Normal },
	{.hex = 40, .name = "Wigglytuff", .type1 = Normal, .type2 = Normal },
	{.hex = 41, .name = "Zubat", .type1 = Poison, .type2 = Flying },
	{.hex = 42, .name = "Golbat", .type1 = Poison, .type2 = Flying },
	{.hex = 43, .name = "Oddish", .type1 = Grass, .type2 = Poison },
	{.hex = 44, .name = "Gloom", .type1 = Grass, .type2 = Poison },
	{.hex = 45, .name = "Vileplume", .type1 = Grass, .type2 = Poison },
	{.hex = 46, .name = "Paras", .type1 = Bug, .type2 = Grass },
	{.hex = 47, .name = "Parasect", .type1 = Bug, .type2 = Grass },
	{.hex = 48, .name = "Venonat", .type1 = Bug, .type2 = Poison },
	{.hex = 49, .name = "Venomoth", .type1 = Bug, .type2 = Poison },
	{.hex = 50, .name = "Diglett", .type1 = Ground, .type2 = Ground },
	{.hex = 51, .name = "Dugtrio", .type1 = Ground, .type2 = Ground },
	{.hex = 52, .name = "Meowth", .type1 = Normal, .type2 = Normal },
	{.hex = 53, .name = "Persian", .type1 = Normal, .type2 = Normal },
	{.hex = 54, .name = "Psyduck", .type1 = Water, .type2 = Water },
	{.hex = 55, .name = "Golduck", .type1 = Water, .type2 = Water },
	{.hex = 56, .name = "Mankey", .type1 = Fighting, .type2 = Fighting },
	{.hex = 57, .name = "Primeape", .type1 = Fighting, .type2 = Fighting },
	{.hex = 58, .name = "Growlithe", .type1 = Fire, .type2 = Fire },
	{.hex = 59, .name = "Arcanine", .type1 = Fire, .type2 = Fire },
	{.hex = 60, .name = "Poliwag", .type1 = Water, .type2 = Water },
	{.hex = 61, .name = "Poliwhirl", .type1 = Water, .type2 = Water },
	{.hex = 62, .name = "Poliwrath", .type1 = Water, .type2 = Fighting },
	{.hex = 63, .name = "Abra", .type1 = Psychic, .type2 = Psychic },
	{.hex = 64, .name = "Kadabra", .type1 = Psychic, .type2 = Psychic },
	{.hex = 65, .name = "Alakazam", .type1 = Psychic, .type2 = Psychic },
	{.hex = 66, .name = "Machop", .type1 = Fighting, .type2 = Fighting },
	{.hex = 67, .name = "Machoke", .type1 = Fighting, .type2 = Fighting },
	{.hex = 68, .name = "Machamp", .type1 = Fighting, .type2 = Fighting },
	{.hex = 69, .name = "Bellsprout", .type1 = Grass, .type2 = Poison },
	{.hex = 70, .name = "Weepinbell", .type1 = Grass, .type2 = Poison },
	{.hex = 71, .name = "Victreebel", .type1 = Grass, .type2 = Poison },
	{.hex = 72, .name = "Tentacool", .type1 = Water, .type2 = Poison },
	{.hex = 73, .name = "Tentacruel", .type1 = Water, .type2 = Poison },
	{.hex = 74, .name = "Geodude", .type1 = Rock, .type2 = Ground },
	{.hex = 75, .name = "Graveler", .type1 = Rock, .type2 = Ground },
	{.hex = 76, .name = "Golem", .type1 = Rock, .type2 = Ground },
	{.hex = 77, .name = "Ponyta", .type1 = Fire, .type2 = Fire },
	{.hex = 78, .name = "Rapidash", .type1 = Fire, .type2 = Fire },
	{.hex = 79, .name = "Slowpoke", .type1 = Water, .type2 = Psychic },
	{.hex = 80, .name = "Slowbro", .type1 = Water, .type2 = Psychic },
	{.hex = 81, .name = "Magnemite", .type1 = Electric, .type2 = Steel },
	{.hex = 82, .name = "Magneton", .type1 = Electric, .type2 = Steel },
	{.hex = 83, .name = "Farfetch", .type1 = Normal, .type2 = Flying },
	{.hex = 84, .name = "Doduo", .type1 = Normal, .type2 = Flying },
	{.hex = 85, .name = "Dodrio", .type1 = Normal, .type2 = Flying },
	{.hex = 86, .name = "Seel", .type1 = Water, .type2 = Water },
	{.hex = 87, .name = "Dewgong", .type1 = Water, .type2 = Ice },
	{.hex = 88, .name = "Grimer", .type1 = Poison, .type2 = Poison },
	{.hex = 89, .name = "Muk", .type1 = Poison, .type2 = Poison },
	{.hex = 90, .name = "Shellder", .type1 = Water, .type2 = Water },
	{.hex = 91, .name = "Cloyster", .type1 = Water, .type2 = Ice },
	{.hex = 92, .name = "Gastly", .type1 = Ghost, .type2 = Poison },
	{.hex = 93, .name = "Haunter", .type1 = Ghost, .type2 = Poison },
	{.hex = 94, .name = "Gengar", .type1 = Ghost, .type2 = Poison },
	{.hex = 95, .name = "Onix", .type1 = Rock, .type2 = Ground },
	{.hex = 96, .name = "Drowzee", .type1 = Psychic, .type2 = Psychic },
	{.hex = 97, .name = "Hypno", .type1 = Psychic, .type2 = Psychic },
	{.hex = 98, .name = "Krabby", .type1 = Water, .type2 = Water },
	{.hex = 99, .name = "Kingler", .type1 = Water, .type2 = Water },
	{.hex = 100, .name = "Voltorb", .type1 = Electric, .type2 = Electric },
	{.hex = 101, .name = "Electrode", .type1 = Electric, .type2 = Electric },
	{.hex = 102, .name = "Exeggcute", .type1 = Grass, .type2 = Psychic },
	{.hex = 103, .name = "Exeggutor", .type1 = Grass, .type2 = Psychic },
	{.hex = 104, .name = "Cubone", .type1 = Ground, .type2 = Ground },
	{.hex = 105, .name = "Marowak", .type1 = Ground, .type2 = Ground },
	{.hex = 106, .name = "Hitmonlee", .type1 = Fighting, .type2 = Fighting },
	{.hex = 107, .name = "Hitmonchan", .type1 = Fighting, .type2 = Fighting },
	{.hex = 108, .name = "Lickitung", .type1 = Normal, .type2 = Normal },
	{.hex = 109, .name = "Koffing", .type1 = Poison, .type2 = Poison },
	{.hex = 110, .name = "Weezing", .type1 = Poison, .type2 = Poison },
	{.hex = 111, .name = "Rhyhorn", .type1 = Ground, .type2 = Rock },
	{.hex = 112, .name = "Rhydon", .type1 = Ground, .type2 = Rock },
	{.hex = 113, .name = "Chansey", .type1 = Normal, .type2 = Normal },
	{.hex = 114, .name = "Tangela", .type1 = Grass, .type2 = Grass },
	{.hex = 115, .name = "Kangaskhan", .type1 = Normal, .type2 = Normal },
	{.hex = 116, .name = "Horsea", .type1 = Water, .type2 = Water },
	{.hex = 117, .name = "Seadra", .type1 = Water, .type2 = Water },
	{.hex = 118, .name = "Goldeen", .type1 = Water, .type2 = Water },
	{.hex = 119, .name = "Seaking", .type1 = Water, .type2 = Water },
	{.hex = 120, .name = "Staryu", .type1 = Water, .type2 = Water },
	{.hex = 121, .name = "Starmie", .type1 = Water, .type2 = Psychic },
	{.hex = 122, .name = "Mr. Mime", .type1 = Psychic, .type2 = Psychic },
	{.hex = 123, .name = "Scyther", .type1 = Bug, .type2 = Flying },
	{.hex = 124, .name = "Jynx", .type1 = Ice, .type2 = Psychic },
	{.hex = 125, .name = "Electabuzz", .type1 = Electric, .type2 = Electric },
	{.hex = 126, .name = "Magmar", .type1 = Fire, .type2 = Fire },
	{.hex = 127, .name = "Pinsir", .type1 = Bug, .type2 = Bug },
	{.hex = 128, .name = "Tauros", .type1 = Normal, .type2 = Normal },
	{.hex = 129, .name = "Magikarp", .type1 = Water, .type2 = Water },
	{.hex = 130, .name = "Gyarados", .type1 = Water, .type2 = Flying },
	{.hex = 131, .name = "Lapras", .type1 = Water, .type2 = Ice },
	{.hex = 132, .name = "Ditto", .type1 = Normal, .type2 = Normal },
	{.hex = 133, .name = "Eevee", .type1 = Normal, .type2 = Normal },
	{.hex = 134, .name = "Vaporeon", .type1 = Water, .type2 = Water },
	{.hex = 135, .name = "Jolteon", .type1 = Electric, .type2 = Electric },
	{.hex = 136, .name = "Flareon", .type1 = Fire, .type2 = Fire },
	{.hex = 137, .name = "Porygon", .type1 = Normal, .type2 = Normal },
	{.hex = 138, .name = "Omanyte", .type1 = Rock, .type2 = Water },
	{.hex = 139, .name = "Omastar", .type1 = Rock, .type2 = Water },
	{.hex = 140, .name = "Kabuto", .type1 = Rock, .type2 = Water },
	{.hex = 141, .name = "Kabutops", .type1 = Rock, .type2 = Water },
	{.hex = 142, .name = "Aerodactyl", .type1 = Rock, .type2 = Flying },
	{.hex = 143, .name = "Snorlax", .type1 = Normal, .type2 = Normal },
	{.hex = 144, .name = "Articuno", .type1 = Ice, .type2 = Flying },
	{.hex = 145, .name = "Zapdos", .type1 = Electric, .type2 = Flying },
	{.hex = 146, .name = "Moltres", .type1 = Fire, .type2 = Flying },
	{.hex = 147, .name = "Dratini", .type1 = Dragon, .type2 = Dragon },
	{.hex = 148, .name = "Dragonair", .type1 = Dragon, .type2 = Dragon },
	{.hex = 149, .name = "Dragonite", .type1 = Dragon, .type2 = Flying },
	{.hex = 150, .name = "Mewtwo", .type1 = Psychic, .type2 = Psychic },
	{.hex = 151, .name = "Mew", .type1 = Psychic, .type2 = Psychic },
	{.hex = 152, .name = "Chikorita", .type1 = Grass, .type2 = Grass },
	{.hex = 153, .name = "Bayleef", .type1 = Grass, .type2 = Grass },
	{.hex = 154, .name = "Meganium", .type1 = Grass, .type2 = Grass },
	{.hex = 155, .name = "Cyndaquil", .type1 = Fire, .type2 = Fire },
	{.hex = 156, .name = "Quilava", .type1 = Fire, .type2 = Fire },
	{.hex = 157, .name = "Typhlosion", .type1 = Fire, .type2 = Fire },
	{.hex = 158, .name = "Totodile", .type1 = Water, .type2 = Water },
	{.hex = 159, .name = "Croconaw", .type1 = Water, .type2 = Water },
	{.hex = 160, .name = "Feraligatr", .type1 = Water, .type2 = Water },
	{.hex = 161, .name = "Sentret", .type1 = Normal, .type2 = Normal },
	{.hex = 162, .name = "Furret", .type1 = Normal, .type2 = Normal },
	{.hex = 163, .name = "Hoothoot", .type1 = Normal, .type2 = Flying },
	{.hex = 164, .name = "Noctowl", .type1 = Normal, .type2 = Flying },
	{.hex = 165, .name = "Ledyba", .type1 = Bug, .type2 = Flying },
	{.hex = 166, .name = "Ledian", .type1 = Bug, .type2 = Flying },
	{.hex = 167, .name = "Spinarak", .type1 = Bug, .type2 = Poison },
	{.hex = 168, .name = "Ariados", .type1 = Bug, .type2 = Poison },
	{.hex = 169, .name = "Crobat", .type1 = Poison, .type2 = Flying },
	{.hex = 170, .name = "Chinchou", .type1 = Water, .type2 = Electric },
	{.hex = 171, .name = "Lanturn", .type1 = Water, .type2 = Electric },
	{.hex = 172, .name = "Pichu", .type1 = Electric, .type2 = Electric },
	{.hex = 173, .name = "Cleffa", .type1 = Normal, .type2 = Normal },
	{.hex = 174, .name = "Igglybuff", .type1 = Normal, .type2 = Normal },
	{.hex = 175, .name = "Togepi", .type1 = Normal, .type2 = Normal },
	{.hex = 176, .name = "Togetic", .type1 = Normal, .type2 = Flying },
	{.hex = 177, .name = "Natu", .type1 = Psychic, .type2 = Flying },
	{.hex = 178, .name = "Xatu", .type1 = Psychic, .type2 = Flying },
	{.hex = 179, .name = "Mareep", .type1 = Electric, .type2 = Electric },
	{.hex = 180, .name = "Flaaffy", .type1 = Electric, .type2 = Electric },
	{.hex = 181, .name = "Ampharos", .type1 = Electric, .type2 = Electric },
	{.hex = 182, .name = "Bellossom", .type1 = Grass, .type2 = Grass },
	{.hex = 183, .name = "Marill", .type1 = Water, .type2 = Water },
	{.hex = 184, .name = "Azumarill", .type1 = Water, .type2 = Water },
	{.hex = 185, .name = "Sudowoodo", .type1 = Rock, .type2 = Rock },
	{.hex = 186, .name = "Politoed", .type1 = Water, .type2 = Water },
	{.hex = 187, .name = "Hoppip", .type1 = Grass, .type2 = Flying },
	{.hex = 188, .name = "Skiploom", .type1 = Grass, .type2 = Flying },
	{.hex = 189, .name = "Jumpluff", .type1 = Grass, .type2 = Flying },
	{.hex = 190, .name = "Aipom", .type1 = Normal, .type2 = Normal },
	{.hex = 191, .name = "Sunkern", .type1 = Grass, .type2 = Grass },
	{.hex = 192, .name = "Sunflora", .type1 = Grass, .type2 = Grass },
	{.hex = 193, .name = "Yanma", .type1 = Bug, .type2 = Flying },
	{.hex = 194, .name = "Wooper", .type1 = Water, .type2 = Ground },
	{.hex = 195, .name = "Quagsire", .type1 = Water, .type2 = Ground },
	{.hex = 196, .name = "Espeon", .type1 = Psychic, .type2 = Psychic },
	{.hex = 197, .name = "Umbreon", .type1 = Dark, .type2 = Dark },
	{.hex = 198, .name = "Murkrow", .type1 = Dark, .type2 = Flying },
	{.hex = 199, .name = "Slowking", .type1 = Water, .type2 = Psychic },
	{.hex = 200, .name = "Misdreavus", .type1 = Ghost, .type2 = Ghost },
	{.hex = 201, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 202, .name = "Wobbuffet", .type1 = Psychic, .type2 = Psychic },
	{.hex = 203, .name = "Girafarig", .type1 = Normal, .type2 = Psychic },
	{.hex = 204, .name = "Pineco", .type1 = Bug, .type2 = Bug },
	{.hex = 205, .name = "Forretress", .type1 = Bug, .type2 = Steel },
	{.hex = 206, .name = "Dunsparce", .type1 = Normal, .type2 = Normal },
	{.hex = 207, .name = "Gligar", .type1 = Ground, .type2 = Flying },
	{.hex = 208, .name = "Steelix", .type1 = Steel, .type2 = Ground },
	{.hex = 209, .name = "Snubbull", .type1 = Normal, .type2 = Normal },
	{.hex = 210, .name = "Granbull", .type1 = Normal, .type2 = Normal },
	{.hex = 211, .name = "Qwilfish", .type1 = Water, .type2 = Poison },
	{.hex = 212, .name = "Scizor", .type1 = Bug, .type2 = Steel },
	{.hex = 213, .name = "Shuckle", .type1 = Bug, .type2 = Rock },
	{.hex = 214, .name = "Heracross", .type1 = Bug, .type2 = Fighting },
	{.hex = 215, .name = "Sneasel", .type1 = Dark, .type2 = Ice },
	{.hex = 216, .name = "Teddiursa", .type1 = Normal, .type2 = Normal },
	{.hex = 217, .name = "Ursaring", .type1 = Normal, .type2 = Normal },
	{.hex = 218, .name = "Slugma", .type1 = Fire, .type2 = Fire },
	{.hex = 219, .name = "Magcargo", .type1 = Fire, .type2 = Rock },
	{.hex = 220, .name = "Swinub", .type1 = Ice, .type2 = Ground },
	{.hex = 221, .name = "Piloswine", .type1 = Ice, .type2 = Ground },
	{.hex = 222, .name = "Corsola", .type1 = Water, .type2 = Rock },
	{.hex = 223, .name = "Remoraid", .type1 = Water, .type2 = Water },
	{.hex = 224, .name = "Octillery", .type1 = Water, .type2 = Water },
	{.hex = 225, .name = "Delibird", .type1 = Ice, .type2 = Flying },
	{.hex = 226, .name = "Mantine", .type1 = Water, .type2 = Flying },
	{.hex = 227, .name = "Skarmory", .type1 = Steel, .type2 = Flying },
	{.hex = 228, .name = "Houndour", .type1 = Dark, .type2 = Fire },
	{.hex = 229, .name = "Houndoom", .type1 = Dark, .type2 = Fire },
	{.hex = 230, .name = "Kingdra", .type1 = Water, .type2 = Dragon },
	{.hex = 231, .name = "Phanpy", .type1 = Ground, .type2 = Ground },
	{.hex = 232, .name = "Donphan", .type1 = Ground, .type2 = Ground },
	{.hex = 233, .name = "Porygon2", .type1 = Normal, .type2 = Normal },
	{.hex = 234, .name = "Stantler", .type1 = Normal, .type2 = Normal },
	{.hex = 235, .name = "Smeargle", .type1 = Normal, .type2 = Normal },
	{.hex = 236, .name = "Tyrogue", .type1 = Fighting, .type2 = Fighting },
	{.hex = 237, .name = "Hitmontop", .type1 = Fighting, .type2 = Fighting },
	{.hex = 238, .name = "Smoochum", .type1 = Ice, .type2 = Psychic },
	{.hex = 239, .name = "Elekid", .type1 = Electric, .type2 = Electric },
	{.hex = 240, .name = "Magby", .type1 = Fire, .type2 = Fire },
	{.hex = 241, .name = "Miltank", .type1 = Normal, .type2 = Normal },
	{.hex = 242, .name = "Blissey", .type1 = Normal, .type2 = Normal },
	{.hex = 243, .name = "Raikou", .type1 = Electric, .type2 = Electric },
	{.hex = 244, .name = "Entei", .type1 = Fire, .type2 = Fire },
	{.hex = 245, .name = "Suicune", .type1 = Water, .type2 = Water },
	{.hex = 246, .name = "Larvitar", .type1 = Rock, .type2 = Ground },
	{.hex = 247, .name = "Pupitar", .type1 = Rock, .type2 = Ground },
	{.hex = 248, .name = "Tyranitar", .type1 = Rock, .type2 = Dark },
	{.hex = 249, .name = "Lugia", .type1 = Psychic, .type2 = Flying },
	{.hex = 250, .name = "Ho-Oh", .type1 = Fire, .type2 = Flying },
	{.hex = 251, .name = "Celebi", .type1 = Psychic, .type2 = Grass },
	{.hex = 252, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 253, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 254, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 255, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 256, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 257, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 258, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 259, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 260, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 261, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 262, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 263, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 264, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 265, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 266, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 267, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 268, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 269, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 270, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 271, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 272, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 273, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 274, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 275, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 276, .name = "?", .type1 = Normal, .type2 = Normal },
	{.hex = 277, .name = "Treecko", .type1 = Grass, .type2 = Grass },
	{.hex = 278, .name = "Grovyle", .type1 = Grass, .type2 = Grass },
	{.hex = 279, .name = "Sceptile", .type1 = Grass, .type2 = Grass },
	{.hex = 280, .name = "Torchic", .type1 = Fire, .type2 = Fire },
	{.hex = 281, .name = "Combusken", .type1 = Fire, .type2 = Fighting },
	{.hex = 282, .name = "Blaziken", .type1 = Fire, .type2 = Fighting },
	{.hex = 283, .name = "Mudkip", .type1 = Water, .type2 = Water },
	{.hex = 284, .name = "Marshtomp", .type1 = Water, .type2 = Ground },
	{.hex = 285, .name = "Swampert", .type1 = Water, .type2 = Ground },
	{.hex = 286, .name = "Poochyena", .type1 = Dark, .type2 = Dark },
	{.hex = 287, .name = "Mightyena", .type1 = Dark, .type2 = Dark },
	{.hex = 288, .name = "Zigzagoon", .type1 = Normal, .type2 = Normal },
	{.hex = 289, .name = "Linoone", .type1 = Normal, .type2 = Normal },
	{.hex = 290, .name = "Wurmple", .type1 = Bug, .type2 = Bug },
	{.hex = 291, .name = "Silcoon", .type1 = Bug, .type2 = Bug },
	{.hex = 292, .name = "Beautifly", .type1 = Bug, .type2 = Flying },
	{.hex = 293, .name = "Cascoon", .type1 = Bug, .type2 = Bug },
	{.hex = 294, .name = "Dustox", .type1 = Bug, .type2 = Poison },
	{.hex = 295, .name = "Lotad", .type1 = Water, .type2 = Grass },
	{.hex = 296, .name = "Lombre", .type1 = Water, .type2 = Grass },
	{.hex = 297, .name = "Ludicolo", .type1 = Water, .type2 = Grass },
	{.hex = 298, .name = "Seedot", .type1 = Grass, .type2 = Grass },
	{.hex = 299, .name = "Nuzleaf", .type1 = Grass, .type2 = Dark },
	{.hex = 300, .name = "Shiftry", .type1 = Grass, .type2 = Dark },
	{.hex = 301, .name = "Nincada", .type1 = Bug, .type2 = Ground },
	{.hex = 302, .name = "Ninjask", .type1 = Bug, .type2 = Flying },
	{.hex = 303, .name = "Shedinja", .type1 = Bug, .type2 = Ghost },
	{.hex = 304, .name = "Taillow", .type1 = Normal, .type2 = Flying },
	{.hex = 305, .name = "Swellow", .type1 = Normal, .type2 = Flying },
	{.hex = 306, .name = "Shroomish", .type1 = Grass, .type2 = Grass },
	{.hex = 307, .name = "Breloom", .type1 = Grass, .type2 = Fighting },
	{.hex = 308, .name = "Spinda", .type1 = Normal, .type2 = Normal },
	{.hex = 309, .name = "Wingull", .type1 = Water, .type2 = Flying },
	{.hex = 310, .name = "Pelipper", .type1 = Water, .type2 = Flying },
	{.hex = 311, .name = "Surskit", .type1 = Bug, .type2 = Water },
	{.hex = 312, .name = "Masquerain", .type1 = Bug, .type2 = Flying },
	{.hex = 313, .name = "Wailmer", .type1 = Water, .type2 = Water },
	{.hex = 314, .name = "Wailord", .type1 = Water, .type2 = Water },
	{.hex = 315, .name = "Skitty", .type1 = Normal, .type2 = Normal },
	{.hex = 316, .name = "Delcatty", .type1 = Normal, .type2 = Normal },
	{.hex = 317, .name = "Kecleon", .type1 = Normal, .type2 = Normal },
	{.hex = 318, .name = "Baltoy", .type1 = Ground, .type2 = Psychic },
	{.hex = 319, .name = "Claydol", .type1 = Ground, .type2 = Psychic },
	{.hex = 320, .name = "Nosepass", .type1 = Rock, .type2 = Rock },
	{.hex = 321, .name = "Torkoal", .type1 = Fire, .type2 = Fire },
	{.hex = 322, .name = "Sableye", .type1 = Dark, .type2 = Ghost },
	{.hex = 323, .name = "Barboach", .type1 = Water, .type2 = Ground },
	{.hex = 324, .name = "Whiscash", .type1 = Water, .type2 = Ground },
	{.hex = 325, .name = "Luvdisc", .type1 = Water, .type2 = Water },
	{.hex = 326, .name = "Corphish", .type1 = Water, .type2 = Water },
	{.hex = 327, .name = "Crawdaunt", .type1 = Water, .type2 = Dark },
	{.hex = 328, .name = "Feebas", .type1 = Water, .type2 = Water },
	{.hex = 329, .name = "Milotic", .type1 = Water, .type2 = Water },
	{.hex = 330, .name = "Carvanha", .type1 = Water, .type2 = Dark },
	{.hex = 331, .name = "Sharpedo", .type1 = Water, .type2 = Dark },
	{.hex = 332, .name = "Trapinch", .type1 = Ground, .type2 = Ground },
	{.hex = 333, .name = "Vibrava", .type1 = Ground, .type2 = Dragon },
	{.hex = 334, .name = "Flygon", .type1 = Ground, .type2 = Dragon },
	{.hex = 335, .name = "Makuhita", .type1 = Fighting, .type2 = Fighting },
	{.hex = 336, .name = "Hariyama", .type1 = Fighting, .type2 = Fighting },
	{.hex = 337, .name = "Electrike", .type1 = Electric, .type2 = Electric },
	{.hex = 338, .name = "Manectric", .type1 = Electric, .type2 = Electric },
	{.hex = 339, .name = "Numel", .type1 = Fire, .type2 = Ground },
	{.hex = 340, .name = "Camerupt", .type1 = Fire, .type2 = Ground },
	{.hex = 341, .name = "Spheal", .type1 = Ice, .type2 = Water },
	{.hex = 342, .name = "Sealeo", .type1 = Ice, .type2 = Water },
	{.hex = 343, .name = "Walrein", .type1 = Ice, .type2 = Water },
	{.hex = 344, .name = "Cacnea", .type1 = Grass, .type2 = Grass },
	{.hex = 345, .name = "Cacturne", .type1 = Grass, .type2 = Dark },
	{.hex = 346, .name = "Snorunt", .type1 = Ice, .type2 = Ice },
	{.hex = 347, .name = "Glalie", .type1 = Ice, .type2 = Ice },
	{.hex = 348, .name = "Lunatone", .type1 = Rock, .type2 = Psychic },
	{.hex = 349, .name = "Solrock", .type1 = Rock, .type2 = Psychic },
	{.hex = 350, .name = "Azurill", .type1 = Normal, .type2 = Normal },
	{.hex = 351, .name = "Spoink", .type1 = Psychic, .type2 = Psychic },
	{.hex = 352, .name = "Grumpig", .type1 = Psychic, .type2 = Psychic },
	{.hex = 353, .name = "Plusle", .type1 = Electric, .type2 = Electric },
	{.hex = 354, .name = "Minun", .type1 = Electric, .type2 = Electric },
	{.hex = 355, .name = "Mawile", .type1 = Steel, .type2 = Steel },
	{.hex = 356, .name = "Meditite", .type1 = Fighting, .type2 = Psychic },
	{.hex = 357, .name = "Medicham", .type1 = Fighting, .type2 = Psychic },
	{.hex = 358, .name = "Swablu", .type1 = Normal, .type2 = Flying },
	{.hex = 359, .name = "Altaria", .type1 = Dragon, .type2 = Flying },
	{.hex = 360, .name = "Wynaut", .type1 = Psychic, .type2 = Psychic },
	{.hex = 361, .name = "Duskull", .type1 = Ghost, .type2 = Ghost },
	{.hex = 362, .name = "Dusclops", .type1 = Ghost, .type2 = Ghost },
	{.hex = 363, .name = "Roselia", .type1 = Grass, .type2 = Poison },
	{.hex = 364, .name = "Slakoth", .type1 = Normal, .type2 = Normal },
	{.hex = 365, .name = "Vigoroth", .type1 = Normal, .type2 = Normal },
	{.hex = 366, .name = "Slaking", .type1 = Normal, .type2 = Normal },
	{.hex = 367, .name = "Gulpin", .type1 = Poison, .type2 = Poison },
	{.hex = 368, .name = "Swalot", .type1 = Poison, .type2 = Poison },
	{.hex = 369, .name = "Tropius", .type1 = Grass, .type2 = Flying },
	{.hex = 370, .name = "Whismur", .type1 = Normal, .type2 = Normal },
	{.hex = 371, .name = "Loudred", .type1 = Normal, .type2 = Normal },
	{.hex = 372, .name = "Exploud", .type1 = Normal, .type2 = Normal },
	{.hex = 373, .name = "Clamperl", .type1 = Water, .type2 = Water },
	{.hex = 374, .name = "Huntail", .type1 = Water, .type2 = Water },
	{.hex = 375, .name = "Gorebyss", .type1 = Water, .type2 = Water },
	{.hex = 376, .name = "Absol", .type1 = Dark, .type2 = Dark },
	{.hex = 377, .name = "Shuppet", .type1 = Ghost, .type2 = Ghost },
	{.hex = 378, .name = "Banette", .type1 = Ghost, .type2 = Ghost },
	{.hex = 379, .name = "Seviper", .type1 = Poison, .type2 = Poison },
	{.hex = 380, .name = "Zangoose", .type1 = Normal, .type2 = Normal },
	{.hex = 381, .name = "Relicanth", .type1 = Rock, .type2 = Water },
	{.hex = 382, .name = "Aron", .type1 = Steel, .type2 = Rock },
	{.hex = 383, .name = "Lairon", .type1 = Steel, .type2 = Rock },
	{.hex = 384, .name = "Aggron", .type1 = Steel, .type2 = Rock },
	{.hex = 385, .name = "Castform", .type1 = Normal, .type2 = Normal },
	{.hex = 386, .name = "Volbeat", .type1 = Bug, .type2 = Bug },
	{.hex = 387, .name = "Illumise", .type1 = Bug, .type2 = Bug },
	{.hex = 388, .name = "Lileep", .type1 = Rock, .type2 = Grass },
	{.hex = 389, .name = "Cradily", .type1 = Rock, .type2 = Grass },
	{.hex = 390, .name = "Anorith", .type1 = Rock, .type2 = Bug },
	{.hex = 391, .name = "Armaldo", .type1 = Rock, .type2 = Bug },
	{.hex = 392, .name = "Ralts", .type1 = Psychic, .type2 = Psychic },
	{.hex = 393, .name = "Kirlia", .type1 = Psychic, .type2 = Psychic },
	{.hex = 394, .name = "Gardevoir", .type1 = Psychic, .type2 = Psychic },
	{.hex = 395, .name = "Bagon", .type1 = Dragon, .type2 = Dragon },
	{.hex = 396, .name = "Shelgon", .type1 = Dragon, .type2 = Dragon },
	{.hex = 397, .name = "Salamence", .type1 = Dragon, .type2 = Flying },
	{.hex = 398, .name = "Beldum", .type1 = Steel, .type2 = Psychic },
	{.hex = 399, .name = "Metang", .type1 = Steel, .type2 = Psychic },
	{.hex = 400, .name = "Metagross", .type1 = Steel, .type2 = Psychic },
	{.hex = 401, .name = "Regirock", .type1 = Rock, .type2 = Rock },
	{.hex = 402, .name = "Regice", .type1 = Ice, .type2 = Ice },
	{.hex = 403, .name = "Registeel", .type1 = Steel, .type2 = Steel },
	{.hex = 404, .name = "Kyogre", .type1 = Water, .type2 = Water },
	{.hex = 405, .name = "Groudon", .type1 = Ground, .type2 = Ground },
	{.hex = 406, .name = "Rayquaza", .type1 = Dragon, .type2 = Flying },
	{.hex = 407, .name = "Latias", .type1 = Dragon, .type2 = Psychic },
	{.hex = 408, .name = "Latios", .type1 = Dragon, .type2 = Psychic },
	{.hex = 409, .name = "Jirachi", .type1 = Steel, .type2 = Psychic },
	{.hex = 410, .name = "Deoxys", .type1 = Psychic, .type2 = Psychic },
	{.hex = 411, .name = "Chimecho", .type1 = Psychic, .type2 = Psychic },
	{.hex = 412, .name = "Pokémon", .type1 = Egg, .type2 = Egg },
	{.hex = 413, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 414, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 415, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 416, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 417, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 418, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 419, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 420, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 421, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 422, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 423, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 424, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 425, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 426, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 427, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 428, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 429, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 430, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 431, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 432, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 433, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 434, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 435, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 436, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 437, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 438, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 439, .name = "Unown", .type1 = Psychic, .type2 = Psychic },
	{.hex = 440, .name = "????????", .type1 = Normal, .type2 = Normal },
};

void dump_team_info(const uint8_t *base, const uint32_t sec_key) {
	enum {
		TEAM_SIZE = 0x234,     // 4
		TEAM_POKEMON = 0x238,  // 600
		MONEY = 0x490,         // 4
		COINS = 0x494,         // 2
		PC_ITEMS = 0x498,      // 200
		BAG_ITEMS = 0x560,     // 80
		KEY_ITEMS = 0x5D8,     // 80
		BALL_ITEMS = 0x650,    // 64
		TM_ITEMS = 0x690,      // 256
		BERRY_ITEMS = 0x790    // 184
	};

	struct TeamInfo {
		uint32_t team_size;
		struct {
			uint32_t personality;
			uint32_t ot_id;
			uint8_t nickname[11]; // +1 for terminator
			uint16_t lang;
			uint8_t ot_name[8];
			uint8_t markings;
			uint16_t checksum;
			uint16_t unknown;
			union {
				//uint8_t data[48];
				struct {
					uint8_t data0[12];
					uint8_t data1[12];
					uint8_t data2[12];
					uint8_t data3[12];
				};
			};
			uint32_t status;
			uint8_t level;
			uint8_t pokerus;
			uint16_t current_hp;
			uint16_t total_hp;
			uint16_t attack;
			uint16_t defense;
			uint16_t speed;
			uint16_t sp_attack;
			uint16_t sp_defense;
		} pokemon[6];
		uint32_t money;
		uint16_t coins;
		struct {
			uint8_t data[200];
		} pc_items;
	} info;

	memcpy(&info.team_size, base + TEAM_SIZE, 4);
	memcpy(&info.pokemon, base + TEAM_POKEMON, 600);
	memcpy(&info.money, base + MONEY, 4);

	for (size_t i = 0; i < info.team_size; i++) {
		const uint8_t *pokemon = base + TEAM_POKEMON + i * 100;
		enum {
			NICKNAME = 8, // 10
			PERSONALITY = 0, // 4
			OT_ID = 4,   // 4
			OT_NAME = 20, // 7
			MARKINGS = 27, // 1
			LEVEL = 84,   // 1
			POKERUS = 85,  // 1
			TRICKY_DATA = 32,
		};

		memcpy(&info.pokemon[i].personality, pokemon + PERSONALITY, 4);
		decode_text(&info.pokemon[i].nickname, pokemon + NICKNAME, 10);
		memcpy(&info.pokemon[i].ot_id, pokemon + OT_ID, 4);
		decode_text(info.pokemon[i].ot_name, pokemon + OT_NAME, 7);
		printf("%s (OT: %s)\n", info.pokemon[i].nickname, info.pokemon[i].ot_name);
		memcpy(&info.pokemon[i].level, pokemon + LEVEL, 1);
		printf("lv %d\n", info.pokemon[i].level);
		//memcpy(&info.pokemon[i].pokerus, pokemon + POKERUS, 1);
		//printf("pokerus remaining %d\n", info.pokemon[i].pokerus);
		union {
			uint8_t data[48];
			struct {
				uint8_t data_g[12];
				uint8_t data_a[12];
				uint8_t data_e[12];
				uint8_t data_m[12];
			};
		} raw_data;

		uint8_t order = info.pokemon[i].personality % 24;
		switch (order) {
			case  0: // GAEM
			case  1: // GAME
			case  2: // GEAM
			case  3: // GEMA
			case  4: // GMAE
			case  5: // GMEA
				memcpy(raw_data.data_g, pokemon + TRICKY_DATA, 12);
				break;
			case  6: // AGEM
			case  7: // AGME
			case  8: // AEGM
			case  9: // AEMG
			case 10: // AMGE
			case 11: // AMEG
				memcpy(raw_data.data_a, pokemon + TRICKY_DATA, 12);
				break;
			case 12: // EGAM
			case 13: // EGMA
			case 14: // EAGM
			case 15: // EAMG
			case 16: // EMGA
			case 17: // EMAG
				memcpy(raw_data.data_e, pokemon + TRICKY_DATA, 12);
				break;
			case 18: // MGAE
			case 19: // MGEA
			case 20: // MAGE
			case 21: // MAEG
			case 22: // MEGA
			case 23: // MEAG
				memcpy(raw_data.data_m, pokemon + TRICKY_DATA, 12);
				break;
			default: break;
		}

		switch (order) {
			case  6: // AGEM
			case  7: // AGME
			case 12: // EGAM
			case 13: // EGMA
			case 18: // MGAE
			case 19: // MGEA
				memcpy(raw_data.data_g, pokemon + TRICKY_DATA + 12, 12);
				break;
			case  0: // GAEM
			case  1: // GAME
			case 14: // EAGM
			case 15: // EAMG
			case 21: // MAEG
			case 20: // MAGE
				memcpy(raw_data.data_a, pokemon + TRICKY_DATA + 12, 12);
				break;
			case  2: // GEAM
			case  3: // GEMA
			case  8: // AEGM
			case  9: // AEMG
			case 22: // MEGA
			case 23: // MEAG
				memcpy(raw_data.data_e, pokemon + TRICKY_DATA + 12, 12);
				break;
			case  4: // GMAE
			case  5: // GMEA
			case 10: // AMGE
			case 11: // AMEG
			case 16: // EMGA
			case 17: // EMAG
				memcpy(raw_data.data_m, pokemon + TRICKY_DATA + 12, 12);
				break;
			default: break;
		}

		switch (order) {
			case 14: // EAGM
			case 20: // MAGE
			case  8: // AEGM
			case 22: // MEGA
			case 10: // AMGE
			case 16: // EMGA
				memcpy(raw_data.data_g, pokemon + TRICKY_DATA + 24, 12);
				break;
			case 12: // EGAM
			case 18: // MGAE
			case  2: // GEAM
			case  4: // GMAE
			case 23: // MEAG
			case 17: // EMAG
				memcpy(raw_data.data_a, pokemon + TRICKY_DATA + 24, 12);
				break;
			case  6: // AGEM
			case 19: // MGEA
			case  0: // GAEM
			case 21: // MAEG
			case  5: // GMEA
			case 11: // AMEG
				memcpy(raw_data.data_e, pokemon + TRICKY_DATA + 24, 12);
				break;
			case  7: // AGME
			case 13: // EGMA
			case  1: // GAME
			case 15: // EAMG
			case  3: // GEMA
			case  9: // AEMG
				memcpy(raw_data.data_m, pokemon + TRICKY_DATA + 24, 12);
				break;
			default: break;
		}

		switch (order) {
			case  9: // AEMG
			case 11: // AMEG
			case 15: // EAMG
			case 21: // MAEG
			case 17: // EMAG
			case 23: // MEAG
				memcpy(raw_data.data_g, pokemon + TRICKY_DATA + 36, 12);
				break;
			case  3: // GEMA
			case  5: // GMEA
			case 13: // EGMA
			case 16: // EMGA
			case 19: // MGEA
			case 22: // MEGA
				memcpy(raw_data.data_a, pokemon + TRICKY_DATA + 36, 12);
				break;
			case  1: // GAME
			case  4: // GMAE
			case  7: // AGME
			case 10: // AMGE
			case 18: // MGAE
			case 20: // MAGE
				memcpy(raw_data.data_e, pokemon + TRICKY_DATA + 36, 12);
				break;
			case  0: // GAEM
			case  2: // GEAM
			case  6: // AGEM
			case  8: // AEGM
			case 12: // EGAM
			case 14: // EAGM
				memcpy(raw_data.data_m, pokemon + TRICKY_DATA + 36, 12);
				break;
			default: break;
		}

		uint32_t key = info.pokemon[i].ot_id ^ info.pokemon[i].personality;
		uint32_t *u32_data = raw_data.data;
		for (int i = 0; i < 12; i++) {
			u32_data[i] ^= key;
		}

		uint16_t species;
		memcpy(&species, raw_data.data_g, 2);

		struct Pokemon poke = pokemon_lut[species];

		printf("species %d (%04x), should be a %s\n", species, species, poke.name);
		printf("\n");
	}

	printf("money $%d\n", info.money ^ sec_key);
}

int main(int argc, char **argv) {
	struct stat s;

	if (argc <= 1) {
		fprintf(stderr, "provide a sav file please");
		exit(-1);
	}

	printf("loading %s\n", argv[1]);

	const char *file_name = argv[1];
	int fd = open(file_name, O_RDONLY);
	check(fd < 0, "open %s failed: %s", file_name, strerror(errno));

	int status = fstat(fd, &s);
	check(status < 0, "stat %s failed: %s", file_name, strerror(errno));
	size_t size = s.st_size;

	const uint8_t *mapped = mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
	check(mapped == MAP_FAILED, "mmap %s failed: %s", file_name, strerror(errno));

	const uint8_t *base_saves[] = {
		mapped,
		mapped + 0xE000
	};

	enum {
		OFFSET_SECTION_ID = 0xFF4,
		OFFSET_CHECKSUM = 0xFF6,
		OFFSET_SAVE_INDEX = 0xFFC
	};

	enum {
		TRAINER_INFO,
		TEAM_ITEMS,
		GAME_STATE,
		MISC_DATA,
		RIVAL_INFO,
		PC_A,
		PC_B,
		PC_C,
		PC_D,
		PC_E,
		PC_F,
		PC_G,
		PC_H,
		PC_I,
	};

	uint32_t save_idx_a, save_idx_b;
	memcpy(&save_idx_a, base_saves[0] + OFFSET_SAVE_INDEX, 4);
	memcpy(&save_idx_b, base_saves[1] + OFFSET_SAVE_INDEX, 4);
	const uint8_t *save = (save_idx_a > save_idx_b) ? base_saves[0] : base_saves[1];
	if (save_idx_a > save_idx_b) {
		printf("save A selected\n");
	}
	else {
		printf("save B selected\n");
	}

	struct {
		size_t trainer_info;
		size_t team_items;
		size_t game_state;
		size_t misc_data;
		size_t rival_info;
		size_t pc_box[9];
	} offsets;
	memset(&offsets, 0, sizeof(offsets));

	for (size_t i = 0; i < 14; i++) {
		const size_t offset = i * 4096;
		uint16_t id;
		memcpy(&id, save + offset + OFFSET_SECTION_ID, 2);
		switch (id) {
			case TRAINER_INFO: offsets.trainer_info = offset; break;
			case TEAM_ITEMS: offsets.team_items = offset; break;
			case GAME_STATE: offsets.game_state = offset;  break;
			case MISC_DATA: offsets.misc_data = offset; break;
			case RIVAL_INFO: offsets.rival_info = offset; break;
			case PC_A: offsets.pc_box[0] = offset; break;
			case PC_B: offsets.pc_box[1] = offset; break;
			case PC_C: offsets.pc_box[2] = offset; break;
			case PC_D: offsets.pc_box[3] = offset; break;
			case PC_E: offsets.pc_box[4] = offset; break;
			case PC_F: offsets.pc_box[5] = offset; break;
			case PC_G: offsets.pc_box[6] = offset; break;
			case PC_H: offsets.pc_box[7] = offset; break;
			case PC_I: offsets.pc_box[8] = offset; break;
			default: break;
		};
	}

	uint32_t sec_key;
	memcpy(&sec_key, save + offsets.trainer_info + 0x1F4, 4);

	uint32_t money;
	memcpy(&money, save + offsets.trainer_info + 0x490, 4);

	dump_trainer_info(save + offsets.trainer_info);
	dump_team_info(save + offsets.team_items, sec_key);

	return 0;
}
