import React, { useEffect, useState, useContext, useRef } from 'react';
import { useNavigate, useLocation } from 'react-router-dom';
import { UserContext } from '../UserContext';

const BattleLobby = () => {
  const { state } = useLocation();
  const gameType = state?.gameType ?? "defaultGame";
  const navigate = useNavigate();
  const [waitingMessage, setWaitingMessage] = useState('Waiting for an opponent to join...');
  const [isMatched, setIsMatched] = useState(false);
  const [error, setError] = useState(null);
  const { username } = useContext(UserContext);
  const pollingRef = useRef();

  useEffect(() => {
    let isActive = true;
    pollingRef.current = setInterval(async () => {
      try {
        const response = await fetch(
          `${import.meta.env.VITE_API_URL}/api/start_or_join`,
          {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, gameType }),
          }
        );
        if (!response.ok) throw new Error("Server error");
        
        const { sessionId, isMatched } = await response.json();
        console.log({ sessionId, isMatched });
        if (!isActive) return; // Prevent state updates after unmount

        if (isMatched) {
          clearInterval(pollingRef.current);
          setIsMatched(true);
          setWaitingMessage('Match found! Starting the game…');
          setTimeout(() => {
            if (gameType === "Tic Tac Toe") {
              navigate("/tictak", {
                state: { sessionId, gameType, username }
              });
            
            }
          }, 500); // Short delay for UX
        }
      } catch (err) {
        clearInterval(pollingRef.current);
        setError("Could not connect to the game server. Please try again later.");
      }
    }, 2000);

    return () => {
      isActive = false;
      clearInterval(pollingRef.current);
    };
  }, [username, gameType, navigate]);

  return (
    <div style={styles.wrapper}>
      <div style={styles.card}>
        <h1 style={styles.heading}>⚔️ Battle Lobby</h1>
        {error ? (
          <p style={{ color: 'red' }}>{error}</p>
        ) : (
          <>
            <p style={styles.message}>{waitingMessage}</p>
            {!isMatched && <div style={styles.dots}><span>.</span><span>.</span><span>.</span></div>}
          </>
        )}
      </div>
    </div>
  );
};

const styles = {
  wrapper: {
    height: '100vh',
    display: 'flex',
    justifyContent: 'center',
    alignItems: 'center',
    background: 'linear-gradient(135deg, #2c3e50, #3498db)',
  },
  card: {
    backgroundColor: '#fff',
    padding: '40px 60px',
    borderRadius: '20px',
    boxShadow: '0 10px 25px rgba(0, 0, 0, 0.2)',
    textAlign: 'center',
    fontFamily: 'Arial, sans-serif',
    color: '#2c3e50',
  },
  heading: {
    fontSize: '2.2rem',
    marginBottom: '20px',
  },
  message: {
    fontSize: '1.2rem',
    marginBottom: '20px',
  },
  dots: {
    fontSize: '2rem',
    animation: 'blink 1.2s infinite',
    display: 'flex',
    justifyContent: 'center',
    gap: '5px',
  },
};

// CSS animation via keyframes injection
const styleSheet = document.styleSheets[0];
const keyframes = `
@keyframes blink {
  0% { opacity: 0.2; }
  50% { opacity: 1; }
  100% { opacity: 0.2; }
}
`;
styleSheet.insertRule(keyframes, styleSheet.cssRules.length);

export default BattleLobby;
