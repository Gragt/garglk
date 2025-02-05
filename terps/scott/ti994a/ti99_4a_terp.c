#include <string.h>

#include "glk.h"
#include "scott.h"

#include "load_ti99_4a.h"
#include "ti99_4a_terp.h"

static ActionResultType PerformTI99Line(const uint8_t *action_line)
{
    if (action_line == NULL)
        return ACT_FAILURE;

    const uint8_t *ptr = action_line;
    int run_code = 0;
    int index = 0;
    ActionResultType result = ACT_FAILURE;
    int opcode, param;

    int try_index;
    int try[32];

    try_index = 0;

    while (run_code == 0) {
        opcode = *(ptr++);

        switch (opcode) {
        case 183: /* is p in inventory? */
#ifdef DEBUG_ACTIONS
            debug_print("Does the player carry %s?\n", Items[*ptr].Text);
#endif
            if (Items[*(ptr++)].Location != CARRIED) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 184: /* is p in room? */
#ifdef DEBUG_ACTIONS
            debug_print("Is %s in location?\n", Items[*ptr].Text);
#endif
            if (Items[*(ptr++)].Location != MyLoc) {
                run_code = 1;
                result = ACT_FAILURE;
            }

            break;

        case 185: /* is p available? */
#ifdef DEBUG_ACTIONS
            debug_print("Is %s held or in location?\n", Items[*ptr].Text);
#endif
            if (Items[*ptr].Location != CARRIED && Items[*ptr].Location != MyLoc) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 186: /* is p here? */
#ifdef DEBUG_ACTIONS
            debug_print("Is %s NOT in location?\n", Items[*ptr].Text);
#endif
            if (Items[*(ptr++)].Location == MyLoc) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 187: /* is p NOT in inventory? */
#ifdef DEBUG_ACTIONS
            debug_print("Does the player NOT carry %s?\n", Items[*ptr].Text);
#endif
            if (Items[*(ptr++)].Location == CARRIED) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 188: /* is p NOT available? */
#ifdef DEBUG_ACTIONS
            debug_print("Is %s neither carried nor in room?\n", Items[*ptr].Text);
#endif

            if (Items[*ptr].Location == CARRIED || Items[*ptr].Location == MyLoc) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 189: /* is p in play? */
#ifdef DEBUG_ACTIONS
            debug_print("Is %s (%d) in play?\n", Items[*ptr].Text, dv);
#endif
            if (Items[*(ptr++)].Location == 0) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 190: /* Is object p NOT in play? */
#ifdef DEBUG_ACTIONS
            debug_print("Is %s NOT in play?\n", Items[*ptr].Text);
#endif
            if (Items[*(ptr++)].Location != 0) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 191: /* Is player is in room p? */
#ifdef DEBUG_ACTIONS
            debug_print("Is location %s?\n", Rooms[*ptr].Text);
#endif
            if (MyLoc != *(ptr++)) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 192: /* Is player NOT in room p? */
#ifdef DEBUG_ACTIONS
            debug_print("Is location NOT %s?\n", Rooms[*ptr].Text);
#endif
            if (MyLoc == *(ptr++)) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 193: /* Is bitflag p clear? */
#ifdef DEBUG_ACTIONS
            debug_print("Is bitflag %d set?\n", *ptr);
#endif
            if ((BitFlags & (1 << *(ptr++))) == 0) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 194: /* Is bitflag p set? */
#ifdef DEBUG_ACTIONS
            debug_print("Is bitflag %d NOT set?\n", *ptr);
#endif
            if (BitFlags & (1 << *(ptr++))) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 195: /* Does the player carry anything? */
#ifdef DEBUG_ACTIONS
            debug_print("Does the player carry anything?\n");
#endif
            if (CountCarried() == 0) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 196: /* Does the player carry nothing? */
#ifdef DEBUG_ACTIONS
            debug_print("Does the player carry nothing?\n");
#endif
            if (CountCarried()) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 197: /* Is CurrentCounter <= p? */
#ifdef DEBUG_ACTIONS
            debug_print("Is CurrentCounter <= %d?\n", *ptr);
#endif
            if (CurrentCounter > *(ptr++)) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 198: /* Is CurrentCounter > p? */
#ifdef DEBUG_ACTIONS
            debug_print("Is CurrentCounter > %d?\n", *ptr);
#endif
            if (CurrentCounter <= *(ptr++)) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 199: /* Is CurrentCounter == p? */
#ifdef DEBUG_ACTIONS
            debug_print("Is current counter == %d?\n", *ptr);
#endif
            if (CurrentCounter != *(ptr++)) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            break;

        case 200: /* Is item p still in initial room? */
#ifdef DEBUG_ACTIONS
            debug_print("Is %s still in initial room?\n", Items[*ptr].Text);
#endif
            if (Items[*ptr].Location != Items[*ptr].InitialLoc) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 201: /* Has item p been moved? */
#ifdef DEBUG_ACTIONS
            debug_print("Has %s been moved?\n", Items[*ptr].Text);
#endif
            if (Items[*ptr].Location == Items[*ptr].InitialLoc) {
                run_code = 1;
                result = ACT_FAILURE;
            }
            ptr++;
            break;

        case 212: /* clear screen */
            glk_window_clear(Bottom);
            break;

        case 214: /* inv */
            AutoInventory = 1;
            break;

        case 215: /* !inv */
            AutoInventory = 0;
            break;

        case 216:
        case 217:
            break;

        case 218:
            if (try_index >= 32) {
                Fatal("ERROR Hit upper limit on try method.\n");
            }
            try[try_index++] = ptr - action_line + *ptr;
            ptr++;
            break;

        case 219: /* get item */
            if (CountCarried() >= GameHeader.MaxCarry) {
                Output(sys[YOURE_CARRYING_TOO_MUCH]);
                run_code = 1;
                result = ACT_FAILURE;
                break;
            } else {
                Items[*ptr].Location = CARRIED;
            }
            ptr++;
            break;

        case 220: /* drop item */
#ifdef DEBUG_ACTIONS
            debug_print("item %d (\"%s\") is now in location.\n", *ptr,
                Items[*ptr].Text);
#endif
            Items[*(ptr++)].Location = MyLoc;
            should_look_in_transcript = 1;
            break;

        case 221: /* go to room */
#ifdef DEBUG_ACTIONS
            debug_print("player location is now room %d (%s).\n", *ptr,
                Rooms[*ptr].Text);
#endif
            MyLoc = *(ptr++);
            should_look_in_transcript = 1;
            Look();
            break;

        case 222: /* move item p to room 0 */
#ifdef DEBUG_ACTIONS
            debug_print(
                "Item %d (%s) is removed from the game (put in room 0).\n",
                *ptr, Items[*ptr].Text);
#endif
            Items[*(ptr++)].Location = 0;
            break;

        case 223: /* darkness */
            BitFlags |= 1 << DARKBIT;
            break;

        case 224: /* light */
            BitFlags &= ~(1 << DARKBIT);
            break;

        case 225: /* set flag p */
#ifdef DEBUG_ACTIONS
            debug_print("Bitflag %d is set\n", dv);
#endif
            BitFlags |= (1 << *(ptr++));
            break;

        case 226: /* clear flag p */
#ifdef DEBUG_ACTIONS
            debug_print("Bitflag %d is cleared\n", dv);
#endif
            BitFlags &= ~(1 << *(ptr++));
            break;

        case 227: /* set flag 0 */
#ifdef DEBUG_ACTIONS
            debug_print("Bitflag 0 is set\n");
#endif
            BitFlags |= (1 << 0);
            break;

        case 228: /* clear flag 0 */
#ifdef DEBUG_ACTIONS
            debug_print("Bitflag 0 is cleared\n");
#endif
            BitFlags &= ~(1 << 0);
            break;

        case 229: /* die */
#ifdef DEBUG_ACTIONS
            debug_print("Player is dead\n");
#endif
            PlayerIsDead();
            DoneIt();
            result = ACT_GAMEOVER;
            break;

        case 230: /* move item p2 to room p */
            param = *(ptr++);
            PutItemAInRoomB(*(ptr++), param);
            break;

        case 231: /* quit */
            DoneIt();
            return ACT_GAMEOVER;

        case 232: /* print score */
            if (PrintScore() == 1)
                return ACT_GAMEOVER;
            StopTime = 2;
            break;

        case 233: /* list contents of inventory */
            ListInventory(0);
            StopTime = 2;
            break;

        case 234: /* refill lightsource */
            GameHeader.LightTime = LightRefill;
            Items[LIGHT_SOURCE].Location = CARRIED;
            BitFlags &= ~(1 << LIGHTOUTBIT);
            break;

        case 235: /* save */
            SaveGame();
            StopTime = 2;
            break;

        case 236: /* swap items p and p2 around */
            param = *(ptr++);
            SwapItemLocations(param, *(ptr++));
            break;

        case 237: /* move item p to the inventory */
#ifdef DEBUG_ACTIONS
            fprintf(stderr,
                "Player now carries item %d (%s).\n",
                *ptr, Items[*ptr].Text);
#endif
            Items[*(ptr++)].Location = CARRIED;
            break;

        case 238: /* make item p same room as item p2 */
            param = *(ptr++);
            MoveItemAToLocOfItemB(param, *(ptr++));
            break;

        case 239: /* nop */
            break;

        case 240: /* look at room */
            Look();
            should_look_in_transcript = 1;
            break;

        case 241: /* unknown */
            break;

        case 242: /* add 1 to current counter */
            CurrentCounter++;
            break;

        case 243: /* sub 1 from current counter */
            if (CurrentCounter >= 1)
                CurrentCounter--;
            break;

        case 244: /* print current counter */
            OutputNumber(CurrentCounter);
            Output(" ");
            break;

        case 245: /* set current counter to p */
#ifdef DEBUG_ACTIONS
            debug_print("CurrentCounter is set to %d.\n", dv);
#endif
            CurrentCounter = *(ptr++);
            break;

        case 246: /*  add to current counter */
#ifdef DEBUG_ACTIONS
            fprintf(stderr,
                "%d is added to currentCounter. Result: %d\n",
                *ptr, CurrentCounter + *ptr);
#endif
            CurrentCounter += *(ptr++);
            break;

        case 247: /* sub from current counter */
            CurrentCounter -= *(ptr++);
            if (CurrentCounter < -1)
                CurrentCounter = -1;
            break;

        case 248: /* go to stored location */
            GoToStoredLoc();
            break;

        case 249: /* swap room and counter */
            SwapLocAndRoomflag(*(ptr++));
            break;

        case 250: /* swap current counter */
            SwapCounters(*(ptr++));
            break;

        case 251: /* print noun */
            PrintNoun();
            break;

        case 252: /* print noun + newline */
            PrintNoun();
            Output("\n");
            break;

        case 253: /* print newline */
            Output("\n");
            break;

        case 254: /* delay */
            Delay(1);
            break;

        case 255: /* end of code block. */
            result = 0;
            run_code = 1;
            try_index = 0; /* drop out of all try blocks! */
            break;

        default:
            if (opcode <= 182 && opcode <= GameHeader.NumMessages + 1) {
                PrintMessage(opcode);
            } else {
                debug_print("Unknown action %d [Param begins %d %d]\n", opcode, action_line[ptr - action_line], action_line[ptr - action_line + 1]);
                break;
            }
            break;
        }

        /* we are on the 0xff opcode, or have fallen through */
        if (run_code == 1 && try_index > 0) {
            if (opcode == 0xff) {
                run_code = 1;
            } else {
                /* dropped out of TRY block */
                /* or at end of TRY block */
                index = try[try_index - 1];

                try_index -= 1;
                try[try_index] = 0;
                run_code = 0;
                ptr = action_line + index;
            }
        }
    }

    return result;
}

void RunImplicitTI99Actions(void)
{
    int probability;
    uint8_t *ptr;
    int loop_flag;

    ptr = ti99_implicit_actions;
    loop_flag = 0;

    /* bail if no auto acts in the game. */
    if (*ptr == 0x0)
        loop_flag = 1;

    while (loop_flag == 0) {
        /*
         p + 0 = chance of happening
         p + 1 = size of code chunk
         p + 2 = start of code
         */

        probability = ptr[0];

        if (RandomPercent(probability))
            PerformTI99Line(ptr + 2);

        if (ptr[1] == 0 || ptr - ti99_implicit_actions >= ti99_implicit_extent)
            loop_flag = 1;

        /* skip code chunk */
        ptr += 1 + ptr[1];
    }
}

/* parses verb noun actions */
ExplicitResultType RunExplicitTI99Actions(int verb_num, int noun_num)
{
    uint8_t *p;
    ExplicitResultType flag = ER_NO_RESULT;
    int match = 0;
    ActionResultType runcode;

    p = VerbActionOffsets[verb_num];

    /* process all code blocks for this verb
     until success or end. */

    while (flag == ER_NO_RESULT) {
        /* we match VERB NOUN or VERB ANY */
        if (p != NULL && (p[0] == noun_num || p[0] == 0)) {
            match = 1;
            runcode = PerformTI99Line(p + 2);

            if (runcode == ACT_SUCCESS) {
                return ER_SUCCESS;
            } else { /* failure */
                if (p[1] == 0)
                    flag = ER_RAN_ALL_LINES;
                else
                    p += 1 + p[1];
            }
        } else {
            if (p == NULL || p[1] == 0)
                flag = ER_RAN_ALL_LINES_NO_MATCH;
            else
                p += 1 + p[1];
        }
    }

    if (match)
        flag = ER_RAN_ALL_LINES;

    return flag;
}
