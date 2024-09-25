# Multithreaded Pizza Order Management System

## Overview

This project implements a multithreaded pizza order management system in C using POSIX threads (pthreads). The system simulates the operations of a pizza restaurant, managing multiple orders simultaneously and ensuring efficient use of limited resources like telephonists, cooks, ovens, and delivery drivers.

The program handles each order from placement to delivery while collecting and reporting statistics on the restaurant's performance.

## Features

- **Multithreading**: Processes multiple pizza orders concurrently using POSIX threads.
- **Resource Management**: Utilizes mutexes and condition variables to synchronize access to shared resources.
- **Order Lifecycle Management**: Manages the entire lifecycle of an order, including placement, preparation, baking, and delivery.
- **Performance Statistics**: Collects and reports on various metrics such as total revenue, number of successful and failed orders, and average service times.

## File Structure

### pizza.h

- **Resource Definitions**: Specifies the number of telephonists, cooks, ovens, and delivery drivers.
- **Time Parameters**: Defines time ranges for tasks such as order processing, payment, preparation, baking, and delivery.
- **Probability and Costs**: Includes probabilities for pizza types (e.g., Margherita, Pepperoni, Special) and their respective costs.
- **Structures**:
  - **Order Structure**: Represents an order with fields for order ID, number of pizzas, types of pizzas, and order time.
- **Function Prototypes**: Declares key functions, including `handle_order` for processing orders and utility functions for time management and statistics collection.

### pizza.c

- **Order Processing**: Implements the `handle_order` function, which manages the stages of an order from placement to delivery.
- **Multithreading**: Creates threads for each order, allowing simultaneous processing of multiple orders.
- **Synchronization**: Uses mutexes and condition variables to ensure that shared resources like telephonists and ovens are accessed safely.
- **Statistics Collection**: Tracks important metrics, including total revenue, number of failed orders, and service times.
- **Main Function**:
  - Initializes resources and starts the threads for order processing.
  - Waits for all threads to complete and then outputs the final statistics.

## How It Works

1. **Initialization**: The program initializes shared resources and sets up mutexes and condition variables.
2. **Order Processing**: For each customer, a thread is created to handle the order. The order goes through several stages: placement, preparation, baking, and delivery.
3. **Synchronization**: Mutexes and condition variables ensure that resources are used efficiently without conflicts.
4. **Statistics**: Throughout its execution, the program collects statistics on performance, which are displayed after all orders are processed.

## Example Execution

To run the program, ensure you have a C environment with POSIX thread support. Compile and execute the program with parameters specifying the number of customers and a random seed for generating orders.

The program will simulate the pizza order management process and print out detailed statistics at the end.

