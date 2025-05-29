import React, { useEffect, useState, useContext, use } from "react";
import { useNavigate } from "react-router-dom";
import { UserContext } from "../UserContext";
import GameSelection from "../GameSelection";
import "./Lobby.css";

const Lobby = () => {
  const [users, setUsers] = useState([]);
  const { username, setUsername } = useContext(UserContext);
  const navigate = useNavigate();

  useEffect(() => {
  // Function to fetch online users
  const fetchUsers = () => {
    fetch(`${import.meta.env.VITE_API_URL}/api/lobby`)
      .then((response) => response.json())
      .then((data) => {
        if (data && data.users) setUsers(data.users);
        else setUsers([]);
      })
      .catch((error) => {
        console.error("Error fetching lobby data:", error);
      });
  };

  fetchUsers(); // Fetch immediately on mount

  // Set up polling every 5 seconds
  const intervalId = setInterval(fetchUsers, 5000);

  // Cleanup
  return () => clearInterval(intervalId);
}, []);

  useEffect(() => {
    if (!username) return;
    const sendHeartbeat = () => {
      fetch(`${import.meta.env.VITE_API_URL}/api/heartbeat`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username }),
      }).catch((error) => {
        console.error("Error sending heartbeat:", error);
      });
    };
    sendHeartbeat();
    const intervalId = setInterval(sendHeartbeat, 30000); // Send heartbeat every 30 seconds
    return () => clearInterval(intervalId); // Cleanup on unmount
  }
  , [username]);


  // Remove user on sign out
  const handleSignOut = async () => {
    try {
      await fetch(`${import.meta.env.VITE_API_URL}/api/logout`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username: String(username) }),
      });
    } catch (err) {}
    setUsername("");
    navigate("/");
  };

  // // Remove user on page unload
  // useEffect(() => {
  //   const handleUnload = () => {
  //     if (username) {
  //       navigator.sendBeacon(
  //         `${import.meta.env.VITE_API_URL}/api/logout`,
  //         JSON.stringify({ username })
  //       );
  //     }
  //   };
  //   window.addEventListener("beforeunload", handleUnload);
  //   return () => {
  //     window.removeEventListener("beforeunload", handleUnload);

  //   };
  // }, [username]);

  return (
    <div className="lobby-bg">
      <div className="lobby-container">
        <button
          className="lobby-signout-btn"
          onClick={handleSignOut}
          title="Sign Out"
        >
          🚪 Sign Out
        </button>
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
