/* config.h
 *
 * Global constants for backhand.
 *
 * Author:  Alastair Hughes
 * Contact: hobbitalastair at yandex dot com
 */

/* SLEEP_INTERVAL is the number of seconds to sleep between retrying failed
 * systems calls where retrying has some hope of success.
 */
#define SLEEP_INTERVAL 1

/* CHILD_RATELIMIT is the minimum number of seconds between each attempt at
 * launching a child.
 *
 * This provides a simple mechanism to avoid trying to restart a broken
 * process constantly.
 */
#define CHILD_RATELIMIT 10

/* CHILD_TIMEOUT is the number of seconds to wait for a child to respond to a
 * SIGTERM before sending it a SIGKILL.
 */
#define CHILD_TIMEOUT 10

/* SOCK_PATHLEN is the maximum path length for bound sockets.
 * Based on the Linux accept() man page 92 should be a safe value for this.
 */
#define SOCK_PATHLEN 92

