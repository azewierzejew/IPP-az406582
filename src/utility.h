/** @file
 * Proste komponenty ogólnego użytku.
 *
 * @author Antoni Żewierżejew <azewierzejew@gmail.com>
 * @date 14.05.2019
 */

#ifndef DROGI_UTILITY_H
#define DROGI_UTILITY_H

/** Etykieta niepowodzenia, używana w @ref FAIL_IF i @ref FAIL. */
#define FAILURE UTILITY_H_FAILURE_LABEL

/**
 * Jeśli warunek jest spełniony to pójdzie do etykiety odpowiadającej za niepowodzenie funkcji.
 * [do ... while(0)] służy do wymuszenia średnika po makrze.
 */
#define FAIL_IF(condition) do { if (condition) goto FAILURE; } while (0)

/**
 * Idzie do etykiety odpowiadającej za niepowodzenie funkcji.
 * [do ... while(0)] służy do wymuszenia średnika po makrze.
 */
#define FAIL do { goto FAILURE; } while (0)

#endif /* DROGI_UTILITY_H */
