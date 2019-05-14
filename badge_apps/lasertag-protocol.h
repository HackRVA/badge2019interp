
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
 * 16 payload bits (4 bits opcode (12-15), and 12 bits operands (0-11))
 *
 */

#define BADGE_IR_GAME_ADDRESS IR_APP1
#define BADGE_IR_BROADCAST_ID 0

#define OPCODE_SET_GAME_START_TIME 0x00
/* Low 12 bits of payload are signed seconds until game starts, up to +/- 34 minutes. */

#define OPCODE_SET_GAME_DURATION 0x01
/* low 12 bits of payload are duration in seconds */

#define OPCODE_HIT 0x02
/* Low 4 bits of payload (bits 0 - 3) are team id of shooter */
/* Bits 4 - 13 are badge ID of shooter */

#define OPCODE_SET_BADGE_TEAM 0x03
/* Low 4 bits of payload are the team ID */

#define OPCODE_REQUEST_BADGE_DUMP 0x04
/*
 * 1. Base station requests info from badge: OPCODE_REQUEST_BADGE_DUMP
 * 2. Badge respondes with OPCODE_BADGE_IDENTITY
 * 3. Badge responds with OPCODE_GAME_ID
 * 4. Badge responds with OPCODE_BADGE_RECORD_COUNT
 * 5. Badge responds with triplets of OPCODE_BADGE_UPLOAD_HIT_RECORD_BADGE_ID,
 *    OPCODE_BADGE_UPLOAD_HIT_RECORD_TIMESTAMP, and OPCODE_SET_BADGE_TEAM.
 */

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

#define OPCODE_BADGE_UPLOAD_HIT_RECORD_TIMESTAMP 0x09
/* 12 bits timestamp of hit, seconds since game start */

#define OPCODE_GAME_ID 0x0a
/* Low 12 bits of payload is unique game ID.  This opcode is bidirectional.  Base
 * station transmits this to the badge at the beginning of a game, and the
 * badge transmits it back to the base station when syncing. */

#define OPCODE_BADGE_IDENTITY 0x0b
/* low 9 bits contain badge id of this badge that is uploading to the base station */

#define OPCODE_BYTE_TEST 0x0c

#define NUM_LASERTAG_POWERUPS 2
#define NUM_VENDOR_BADGES 10
#define OPCODE_VENDOR_POWER_UP 0x0d
#define AUTO_GRANT_POWERUP_INTERVAL 10 /* seconds */
/* Bits 4 - 13 are badge ID of power-up grantor (same as for HIT packet). */
/* There is a fixed mapping of low badge numbers to powerups.
 * So vendor badges need to be in the low-badge number range (say, 1-9)
 */

