# GamesArena Backend – Local Development with Docker Compose

This guide explains how to run the **backend** of GamesArena locally using Docker Compose. Docker Compose will set up your backend application and its database in isolated containers, making local development and testing easy and consistent.

---

## **Prerequisites**

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) installed
- [Docker Compose](https://docs.docker.com/compose/) (comes with Docker Desktop)
- (Optional) Clone this repository using Git

---

## **Project Structure**

/
├── backend/
│ ├── Dockerfile
│ ├── .env
│ └── (source code)
├── docker-compose.yml
└── (other folders, e.g., frontend/)


---

## **Configuration**

### 1. **Environment Variables**

- The backend reads configuration (like database credentials) from the `backend/.env` file.
- Make sure your `backend/.env` includes the required variables, for example:

    ```
    PG_HOST=database
    PG_USER=postgres
    PG_PASSWORD=yourpassword
    PG_DB=gamesarena
    ```

- The `PG_HOST` should match the database service name in `docker-compose.yml` (commonly `database`).

---

### 2. **docker-compose.yml Example**

A typical `docker-compose.yml` for backend + Postgres might look like:

*(Adjust versions, ports, and credentials as needed for your project)*

---

## **How to Run Locally**

1. **Clone the Repository (if you haven’t already):**
    ```
    git clone https://github.com/youruser/gamesarena.git
    cd gamesarena
    ```

2. **Build and Start the Services:**
    ```
    docker-compose up --build
    ```
    - This command builds the backend image and starts both the backend and database containers.
    - The first time may take a few minutes.

3. **Access the Backend API:**
    - The backend will be available at [http://localhost:8080](http://localhost:8080) (or the port you mapped).
    - Example endpoint: `POST http://localhost:8080/api/signup`

4. **Stopping the Services:**
    ```
    docker-compose down
    ```
    - This stops and removes the containers, but preserves database data in the `db_data` volume.

5. **(Optional) Reset Everything:**
    ```
    docker-compose down -v
    ```
    - This removes containers **and** deletes database data.

---

## **Useful Commands**

- **View logs:**  
  `docker-compose logs -f`
- **Rebuild after code changes:**  
  `docker-compose up --build`
- **Restart only backend:**  
  `docker-compose restart backend`
- **Access database shell:**  
  `docker exec -it gamesarena_database_1 psql -U postgres -d gamesarena`

---

## **Troubleshooting**

- Ensure ports `8080` (backend) and `5432` (Postgres) are not in use by other applications.
- If you change environment variables, rebuild with `docker-compose up --build`.
- For persistent database data, Docker manages the `db_data` named volume.

---

## **References**

- [Docker Compose Docs](https://docs.docker.com/compose/gettingstarted/)  
- [Compose for Developers](https://dev.to/yukaty/docker-compose-for-developers-2ll2)

---

**Enjoy developing GamesArena locally!**
