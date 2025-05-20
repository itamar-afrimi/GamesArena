import React, { useState, useContext } from "react";
import { useNavigate } from "react-router-dom";
import { UserContext } from '../UserContext';
import "./LoginSignup.css";

const LoginSignup = () => {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [message, setMessage] = useState("");
  const [success, setSuccess] = useState(false);
  const navigate = useNavigate();
  const { setUsername: setUsernameContext } = useContext(UserContext);

  const handleRequest = async (endpoint) => {
    setMessage(""); setSuccess(false);
    try {
      const response = await fetch(`${import.meta.env.VITE_API_URL}/api/${endpoint}`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ username, password }),
      });
      const data = await response.json();
      setMessage(data.message || (response.ok ? "Success!" : "Failed."));
      setSuccess(response.ok);

      if (response.ok) {
        setUsernameContext(username);
        setTimeout(() => navigate("/lobby"), 900); // Show success for a moment
      }
    } catch (error) {
      setSuccess(false);
      setMessage("Something went wrong.");
    }
  };

  return (
    <div className="login-bg">
      <form
        className="login-container"
        onSubmit={e => { e.preventDefault(); handleRequest("login"); }}
        autoComplete="off"
      >
        <h1 className="login-title">🎮 Games Arena</h1>
        <input
          className="login-input"
          type="text"
          placeholder="Username"
          value={username}
          autoFocus
          autoComplete="username"
          onChange={e => setUsername(e.target.value)}
        />
        <input
          className="login-input"
          type="password"
          placeholder="Password"
          value={password}
          autoComplete="current-password"
          onChange={e => setPassword(e.target.value)}
        />
        <button
          className="login-btn"
          type="submit"
          disabled={!username || !password}
        >
          Log In
        </button>
        <button
          className="login-btn"
          type="button"
          style={{ marginTop: 8, background: "linear-gradient(90deg, #22d3ee 0%, #6366f1 100%)" }}
          disabled={!username || !password}
          onClick={() => handleRequest("signup")}
        >
          Sign Up
        </button>
        <div className={`login-message${success ? " success" : ""}`}>
          {message}
        </div>
      </form>
    </div>
  );
};

export default LoginSignup;
