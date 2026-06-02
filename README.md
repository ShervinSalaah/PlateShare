# plateShare

----
## Features

### User Accounts
- Create account with username, password, full name, and email
- Sign in with username and password
- View all community members
- Update your profile

### Food Sharing
- Add food with name, description, quantity, expiry date
- Choose pickup method: Self Pickup or Open to Delivery
- Include address or delivery area
- Search food by name or donor
- Sort by expiry date, donor name, or pickup method
- View and delete your own food listings

### Food Requests
- Browse available food and make requests
- View requests on your food (filter by pending, accepted, declined)
- Accept or decline requests with confirmation
- View your request history (most recent first)

### Chat & Messages
- Live group chat with all community members
- Send direct messages to specific members
- Individual chat history per conversation
- Inbox (received messages) and Sent folders
- Messages saved in per-user files

### Notifications
- Alerts when someone requests your food
- Alerts when your request is accepted or declined
- Alerts when you receive a direct message
- Alerts when new food is shared in the community
- Notification count shown on main menu
- Alerts marked as read after viewing

### Dashboard
- Your food shared count
- Successfully donated count
- Your requests made count
- Accepted requests count
- Pending requests on your food

---

## Team Contributions

| Member                      | Module              | Responsibility                                             |
|-----------------------------|---------------------|------------------------------------------------------------|
| 254190K SAMARASINGHE S.M.M.P| Core System         | Menu, dashboard, settings, session management              |
| 254187H SAHITHIYAN J        | User Management     | Account creation, sign in, profile, CSV persistence        |
| 254186E RUBASINHEGE S.N     | Plate Management    | Food sharing, search, filter, sort, pickup/delivery        |
| 254189P SALIYA J.F          | Request Management  | Food requests, accept/decline, transaction history         |
| 254188L SALAAH H.S.S        | Chat & integration  | Group chat, direct messages, inbox, alerts, integration    |

---

## Programming Concepts Used

- Structures (5 total: User, Plate, Request, ChatMessage, Config)
- Arrays of structures
- File I/O (CSV read/write for persistence)
- String handling (strcmp, strcpy, strstr, strcspn, snprintf)
- Menu-driven interface using switch
- Bubble sort algorithm
- Linear search algorithm
- Input validation with re-prompt on error
- Date validation and comparison
- File-based LAN data sharing
- Live polling for real-time chat
- Notification system with per-user files

---

## Honest Limitations

- No real concurrency control — only one user should write to files at a time
- Chat is file-based with 1-2 second polling delay, not true socket networking
- Passwords stored in plain text 
- Fixed array sizes (no dynamic memory allocation)
- Designed for a controlled LAN network

---

## Troubleshooting

**"data folder not found"**
- Make sure you run the executable from the project root folder
- The data folder is created automatically on first run

**LAN sharing doesn't work**
- All computers must be on the same network
- Network Discovery and File Sharing must be enabled in Windows

**Chat doesn't update live**
- Messages appear within 1-2 seconds via file polling
- Avoid simultaneous writes from multiple users

**Compilation warnings about snprintf truncation**
- These are harmless warnings about path length limits
- They won't cause any issues with normal folder paths

---

1. Run the program
2. Register 2-3 users 
3. Sign in as one user and add a few food plates
4. Sign in as another user and make some requests
5. Use the chat to send some messages

All data persists between runs automatically.

---

Enjoy sharing food with your community!
