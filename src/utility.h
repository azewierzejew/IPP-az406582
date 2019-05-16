/** @file
 * Proste komponenty ogólnego użytku.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 14.05.2019
 */

#ifndef __DROGI_UTILITY_H__
#define __DROGI_UTILITY_H__

/** Etykieta niepowodzenia funkcji. */
#define FAILURE UTILITY_H_FAILURE_LABEL

/**
 * Jeśli warunek jest spełniony to pójdzie do etykiety odpowiadającej za niepowodzenie funkcji.
 * [do ... while(0)] służy do wymuszenia średnika po makrze.
 */
#define FAIL_IF(x) do { if (x) goto FAILURE; } while (0)

/**
 * Idzie do etykiety odpowiadającej za niepowodzenie funkcji.
 * [do ... while(0)] służy do wymuszenia średnika po makrze.
 */
#define FAIL do { goto FAILURE; } while (0)

#endif /* __DROGI_UTILITY_H__ */
