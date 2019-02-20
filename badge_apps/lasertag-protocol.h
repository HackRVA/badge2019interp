
/* We have 16 bits of payload. Let's say the high order 4 bits are the opcode.
 * That gives us 16 opcodes, with 12 bits of payload per opcode for single
 * packet opcodes (we can have multi-packet opcodes if needed).
 * All integer values are transmitted little endian (low order byte first).
 *
 * Badge packet is 32-bits:
 * 1 start bit
 * 1 cmd bit
 * 5 address bits (like port number)
 * 9 badge id bits
 * 16 payload bits
 *
 */

#define BADGE_IR_GAME_ADDRESS 0x1A /* Arbitrary 5 bits for now ... we will need to coordinate this later */

#define OPCODE_SET_GAME_START_TIME 0x00
/* Low 12 bits of payload are signed seconds until game starts, up to +/- 34 minutes. */

#define OPCODE_SET_GAME_DURATION 0x01
/* low 12 bits of payload are duration in seconds */

#define OPCODE_HIT 0x02
/* Low 4 bits of payload are team id of shooter */

#define OPCODE_SET_BADGE_TEAM 0x03
/* Low 4 bits of payload are the team ID */

#define OPCODE_REQUEST_BADGE_DUMP 0x04

/* Set game variant.  Low 4 bits of payload contain game variant. */
#define OPCODE_SET_GAME_VARIANT 0x05
#define   GAME_VARIANT_FREE_FOR_ALL 0
#define   GAME_VARIANT_TEAM_BATTLE 1
#define   GAME_VARIANT_ZOMBIE 2
#define   GAME_VARIANT_CAPTURE_THE_BADGE 3
#define   GAME_VARIANT_NONE 0x0f

#define OPCODE_BADGE_RECORD_COUNT 0x07
/* low 12 bits contain count of records about to be uploaded to base station */

#define OPCODE_BADGE_UPLOAD_HIT_RECORD_BADGE_ID 0x08
/* low 9 bits contain badge id of shooter */

#define OPCODE_GAME_ID 0x0a
/* payload is 16 bit unique game ID.  This opcode is bidirectional.  Base
 * station transmits this to the badge at the beginning of a game, and the
 * badge transmits it back to the base station when syncing. */

#define OPCODE_BADGE_UPLOAD_HIT_RECORD_TIMESTAMP 0x09
/* 16 bits timestamp of hit, seconds since game start */

