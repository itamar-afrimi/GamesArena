import React, { useEffect, useState } from "react";
import GameSelection from "../GameSelection";
import "./Lobby.css"; // Import the CSS file

const Lobby = () => {
  const [users, setUsers] = useState([]);

  useEffect(() => {
    fetch(`${import.meta.env.VITE_API_URL}/api/lobby`)
      .then((response) => response.json())
      .then((data) => {
        if (data && data.users) {
          setUsers(data.users);
        } else {
          setUsers([]);
        }
      })
      .catch((error) => {
        console.error("Error fetching lobby data:", error);
      });
  }, []);

  return (
    <div className="lobby-bg">
      <div className="lobby-container">
        <h1 className="lobby-title">🎮 Game Lobby</h1>
        <p className="lobby-subtitle">Welcome! You’re now logged in.</p>
        <div className="lobby-section">
          <GameSelection />
        </div>
        <div className="lobby-section">
          <h2 className="lobby-online-title">Online Users</h2>
          <div className="lobby-user-list">
            {users.length === 0 ? (
              <p className="lobby-no-users">No users online.</p>
            ) : (
              users.map((user, index) => (
                <div key={index} className="lobby-user-card">
                  <span role="img" aria-label="user" className="lobby-user-icon">
                    👤
                  </span>
                  <span className="lobby-username">{user}</span>
                </div>
              ))
            )}
          </div>
        </div>
      </div>
    </div>
  );
};

export default Lobby;
