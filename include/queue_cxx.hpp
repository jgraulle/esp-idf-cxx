#pragma once

#ifdef __cpp_exceptions

#include "esp_exception.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <optional>

namespace idf {

template <typename T>
class Queue
{
public:
    /**
     * @brief Creates a new queue instance
     *
     * @note Internally, within the FreeRTOS implementation, queues use two blocks of memory. The
     *       first block is used to hold the queue’s data structures. The second block is used to
     *       hold items placed into the queue. If a queue is created using Queue() then both blocks
     *       of memory are automatically dynamically allocated inside the Queue() constructor. (see
     *       https://www.FreeRTOS.org/a00111.html).
     *
     * @param length The maximum number of items that the queue can contain.
     *
     * @throw
     *      - idf::ESPException(ESP_ERR_NO_MEM) if out of memory
     */
    explicit Queue(std::size_t length)
    {
        _handle = xQueueCreate(length, sizeof(T));
        if (_handle==nullptr)
            throw idf::ESPException(ESP_ERR_NO_MEM);
    }

    /**
     * @brief Delete a queue
     *
     * Freeing all the memory allocated for storing of items placed on the queue.
     */
    ~Queue()
    {
        vQueueDelete(_handle);
    }

    bool isQueueEmptyFromISR() const
    {
        return xQueueIsQueueEmptyFromISR(_handle) == pdTRUE;
    }

    bool isQueueFullFromISR() const
    {
        return xQueueIsQueueFullFromISR(_handle) == pdTRUE;
    }

    /**
     * @brief Return the number of messages stored in a queue.
     */
    std::size_t messagesWaiting() const
    {
        return uxQueueMessagesWaiting(_handle);
    }

    std::size_t messagesWaitingFromISR() const
    {
        return uxQueueMessagesWaitingFromISR(_handle);
    }

    /**
     * @brief Return the number of free spaces available in a queue.
     *
     * This is equal to the number of items that can be sent to the queue before the queue becomes
     * full if no items are removed.
     */
    std::size_t spacesAvailable() const
    {
        return uxQueueSpacesAvailable(_handle);
    }

    /**
     * @brief Post an item to the front of a queue. The item is queued by copy, not by reference.
     *
     * @warning This function must not be called from an interrupt service routine. See
     *          sendFromISR() for an alternative which may be used in an ISR.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue.
     *        The size of the items the queue will hold was defined when the queue was created,
     *        so this many bytes will be copied from itemToQueue into the queue storage area.
     * @param ticksToWait The maximum amount of time the task should block waiting for space to
     *        become available on the queue, should it already be full. The call will return
     *        immediately if this is set to 0 and the queue is full. The time is defined in tick
     *        periods so the constant portTICK_PERIOD_MS should be used to convert to real time if
     *        this is required.
     * @return true if the item was successfully posted, otherwise false.
     */
    bool sendToFront(const T & itemToQueue, TickType_t ticksToWait)
    {
        return xQueueSendToFront(_handle, &itemToQueue, ticksToWait) == pdTRUE;
    }

    /**
     * @brief Post an item to the front of a queue.
     *
     * @note It is safe to use this macro from within an interrupt service routine.
     * @note Items are queued by copy not reference so it is preferable to only queue small items,
     *       especially when called from an ISR. In most cases it would be preferable to store a
     *       pointer to the item being queued.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue.
     *        The size of the items the queue will hold was defined when the queue was created, so
     *        this many bytes will be copied from pvItemToQueue into the queue storage area.
     * @param[out] higherPriorityTaskWoken set to true if sending to the queue caused a task to
     *             unblock, and the unblocked task has a priority higher than the currently running
     *             task. If sendToFromFromISR() sets this value to true then a context switch should
     *             be requested before the interrupt is exited.
     * @return true if the data was successfully sent to the queue, otherwise false.
     */
    bool sendToFrontFromISR(const T & itemToQueue, bool & higherPriorityTaskWoken)
    {
        BaseType_t temp = pdFALSE;
        if (xQueueSendToFrontFromISR(_handle, &itemToQueue, &temp) != pdTRUE)
            return false;
        if (temp == pdTRUE)
            higherPriorityTaskWoken = true;
        return true;
    }

    /**
     * @brief Post an item to the back of a queue
     *
     * @note The item is queued by copy, not by reference.
     * @warning This function must not be called from an interrupt service routine. See
     *          sendFromISR() for an alternative which may be used in an ISR.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue. The size
     *        of the items the queue will hold was defined when the queue was created, so this many
     *        bytes will be copied from itemToQueue into the queue storage area.
     * @param ticksToWait The maximum amount of time the task should block waiting for space to
     *        become available on the queue, should it already be full. The call will return
     *        immediately if this is set to 0 and the queue is full. The time is defined in tick
     *        periods so the constant portTICK_PERIOD_MS should be used to convert to real time if
     *        this is required.
     * @param true if the item was successfully posted, otherwise false.
     */
    bool sendToBack(const T & itemToQueue, TickType_t ticksToWait)
    {
        return xQueueSendToBack(_handle, &itemToQueue, ticksToWait) == pdTRUE;
    }

    /**
     * @brief Post an item to the back of a queue
     *
     * @note It is safe to use this macro from within an interrupt service routine.
     * @note Items are queued by copy not reference so it is preferable to only queue small items,
     *       especially when called from an ISR. In most cases it would be preferable to store a
     *       pointer to the item being queued.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue. The size
     *        of the items the queue will hold was defined when the queue was created, so this many
     *        bytes will be copied from pvItemToQueue into the queue storage area.
     * @param[out] higherPriorityTaskWoken set to true if sending to the queue caused a task to
     *             unblock, and the unblocked task has a priority higher than the currently running
     *             task. If sets this value to true then a context switch should be requested before
     *             the interrupt is exited, for exemple return true in a GpTimer::EventCallBack
     * @return true if the data was successfully sent to the queue, otherwise false.
     */
    bool sendToBackFromISR(const T & itemToQueue, bool & higherPriorityTaskWoken)
    {
        BaseType_t temp = pdFALSE;
        if (xQueueSendToBackFromISR(_handle, &itemToQueue, &temp) != pdTRUE)
            return false;
        if (temp == pdTRUE)
            higherPriorityTaskWoken = true;
        return true;
    }

    /**
     * @brief Post an item on a queue.
     *
     * @note The item is queued by copy, not by reference.
     * @note It is included for backward compatibility with versions of FreeRTOS.org that did not
     *       include the sendToFront() and SendToBack() macros. It is equivalent to SendToBack().
     * @warning This function must not be called from an interrupt service routine. See
     *          sendFromISR() for an alternative which may be used in an ISR.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue. The size
     *        of the items the queue will hold was defined when the queue was created, so this many
     *        bytes will be copied from itemToQueue into the queue storage area.
     * @param ticksToWait The maximum amount of time the task should block waiting for space to
     *        become available on the queue, should it already be full. The call will return
     *        immediately if this is set to 0 and the queue is full. The time is defined in tick
     *        periods so the constant portTICK_PERIOD_MS should be used to convert to real time if
     *        this is required.
     * @return true if the item was successfully posted, otherwise false.
     */
    bool send(const T & itemToQueue, TickType_t ticksToWait)
    {
        return xQueueSend(_handle, &itemToQueue, ticksToWait) == pdTRUE;
    }

    /**
     * @brief Post an item to the back of a queue
     *
     * @note It is included for backward compatibility with versions of FreeRTOS.org that did not
     *       include the sendToBackFromISR() and sendToFrontFromISR().
     * @note It is safe to use this function from within an interrupt service routine.
     * @note Items are queued by copy not reference so it is preferable to only queue small items,
     *       especially when called from an ISR. In most cases it would be preferable to store a
     *       pointer to the item being queued.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue. The size
     *        of the items the queue will hold was defined when the queue was created, so this many
     *        bytes will be copied from pvItemToQueue into the queue storage area.
     * @param[out] higherPriorityTaskWoken set to true if sending to the queue caused a task to
     *             unblock, and the unblocked task has a priority higher than the currently running
     *             task. If sets this value to true then a context switch should be requested before
     *             the interrupt is exited, for exemple return true in a GpTimer::EventCallBack
     * @return true if the data was successfully sent to the queue, otherwise false.
     */
    bool sendFromISR(const T & itemToQueue, bool & higherPriorityTaskWoken)
    {
        BaseType_t temp = pdFALSE;
        if (xQueueSendFromISR(_handle, &itemToQueue, &temp) != pdTRUE)
            return false;
        if (temp == pdTRUE)
            higherPriorityTaskWoken = true;
        return true;
    }

    /**
     * @brief Post an item on a queue. If the queue is already full then overwrite the value held in the
     * queue.
     *
     * @warning Only for use with queues that have a length of one - so the queue is either empty
     *          or full.
     * @note The item is queued by copy, not by reference.
     * @warning This function must not be called from an interrupt service routine. See
     *          overwriteFromISR() for an alternative which may be used in an ISR.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue. The size
     *        of the items the queue will hold was defined when the queue was created, so this many
     *        bytes will be copied from itemToQueue into the queue storage area.
     */
    void overwrite(const T & itemToQueue)
    {
        assert(xQueueOverwrite(_handle, itemToQueue) == pdPASS);
    }

    /**
     * @brief Post an item on a queue. If the queue is already full then overwrite the value held
     *        in the queue.
     *
     * @warning Only for use with queues that can hold a single item - so the queue is either empty
     *          or full.
     * @note A version of overwrite() that can be used in an interrupt service routine (ISR).
     * @note The item is queued by copy, not by reference.
     *
     * @param itemToQueue A const reference to the item that is to be placed on the queue.
     *        The size of the items the queue will hold was defined when the queue was created,
     *        so this many bytes will be copied from pvItemToQueue into the queue storage area.
     * @param[out] higherPriorityTaskWoken set to true if sending to the queue caused a task to
     *             unblock, and the unblocked task has a priority higher than the currently running
     *             task. If sets this value to true then a context switch should be requested before
     *             the interrupt is exited.
     */
    void overwriteFromISR(const T & itemToQueue, bool & higherPriorityTaskWoken)
    {
        BaseType_t temp = pdFALSE;
        assert(xQueueOverwriteFromISR(_handle, &itemToQueue, &temp) == pdPASS);
        if (temp == pdTRUE)
            higherPriorityTaskWoken = true;
    }

    /**
     * @brief Receive an item from a queue without removing the item from the queue.
     *
     * @note Successfully received items remain on the queue so will be returned again by the next
     *       call, or a call to receive().
     * @warning This function must not be used in an interrupt service routine. See peekFromISR()
     *          for an alternative that can be called from an interrupt service routine.
     *
     * @param ticksToWait The maximum amount of time the task should block waiting for an item to
     *        receive should the queue be empty at the time of the call. The time is defined in tick
     *        periods so the constant portTICK_PERIOD_MS should be used to convert to real time if
     *        this is required. peek() will return immediately if ticksToWait is 0 and the queue is
     *        empty.
     * @return A copy of the item if the queue not empty
     */
    std::optional<T> peek(TickType_t ticksToWait) const
    {
        T value;
        if (xQueuePeek(_handle, &value, ticksToWait) != pdTRUE)
            return {};
        return value;
    }

    /**
     * @brief Receive an item from a queue without removing the item from the queue.
     *
     * @note A version of peek() that can be called from an interrupt service routine (ISR).
     * @note Successfully received items remain on the queue so will be returned again by the next
     *       call, or a call to receive().
     *
     * @return A copy of the item if the queue not empty
     */
    std::optional<T> peekFromISR() const
    {
        T value;
        if (pdTRUE != xQueuePeekFromISR(_handle, &value))
            return {};
        return value;
    }

    /**
     * @brief Receive an item from a queue.
     *
     * @note Successfully received items are removed from the queue.
     * @warning This function must not be used in an interrupt service routine. See receiveFromISR
     *          for an alternative that can.
     *
     * @param ticksToWait The maximum amount of time the task should block waiting for an item to
     *        receive should the queue be empty at the time of the call. receive() will return
     *        immediately if ticksToWait is zero and the queue is empty. The time is defined in tick
     *        periods so the constant portTICK_PERIOD_MS should be used to convert to real time if
     *        this is required.
     * @return A copy of the item if the queue not empty
     */
    std::optional<T> receive(TickType_t ticksToWait)
    {
        T value;
        if (xQueueReceive(_handle, &value, ticksToWait) != pdTRUE)
            return {};
        return value;
    }

    /**
     * @brief Receive an item from a queue.
     *
     * @note It is safe to use this function from within an interrupt service routine.
     *
     * @param[out] taskWokenByReceive A task may be blocked waiting for space to become available
     *             on the queue. If receiveFromISR causes such a task to unblock taskWokenByReceive
     *             will get set to true, otherwise taskWokenByReceive will remain unchanged.
     */
    std::optional<T> receiveFromISR(bool * taskWokenByReceive = nullptr)
    {
        T value;
        BaseType_t temp = pdFALSE;
        if (xQueueReceiveFromISR(_handle, &value, &temp) != pdTRUE)
            return {};
        if (taskWokenByReceive != nullptr && temp == pdTRUE)
            *taskWokenByReceive = true;
        return value;
    }

    /**
     * @brief Reset a queue back to its original empty state.
     */
    void reset()
    {
        // The return value is now obsolete and is always set to pdPASS.
        assert(xQueueReset(_handle) == pdPASS);
    }

private:
    Queue(const Queue &) = delete;
    Queue & operator=(const Queue &) = delete;

    QueueHandle_t _handle;
};

} // idf

#endif // __cpp_exceptions
