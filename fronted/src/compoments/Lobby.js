import React, { useEffect, useState } from "react";
import GameSelection from "../GameSelection"; // Import the GameSelection component



const Lobby = () => {
    const [users, setUsers] = React.useState([]);
  
    React.useEffect(() => {
      fetch("http://localhost:18080/api/lobby")
        .then((response) => response.json())
        .then((data) => {
          if (data && data.users) {
            setUsers(data.users); // Set the users array if it's present
          } else {
            setUsers([]); // In case of no users or invalid data
          }
        })
        .catch((error) => {
          console.error("Error fetching lobby data:", error);
        });
    }, []);
  
    return (
      <div style={styles.container}>
        <h1>Game Lobby</h1>
        <p>Welcome! You’re now logged in.</p>
        {<GameSelection />}
        <div>
          <h2>Online Users:</h2>
          {users.length === 0 ? (
            <p>No users online.</p>
          ) : (
            <ul>
              {users.map((user, index) => (
                <li key={index}>{user}</li>
              ))}
            </ul>
          )}
        </div>
      </div>
    );
  };
  
  const styles = {
    container: {
      marginTop: "100px",
      textAlign: "center",
    },
  };
  
  export default Lobby;
  